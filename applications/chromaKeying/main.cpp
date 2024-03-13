#include <GUI.h>
#include <macros.h>

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
IGNORE_WARNINGS_POP

// STD includes
#include <iostream>

const std::string IMAGES_ROOT = "C:/images";
const std::string RESULTS_ROOT = "C:/images/results";

std::string windowName = "Chroma Keying App";

cv::Mat currentFrame;
cv::Mat resultFrame;
cv::Mat helpImage;
cv::Mat headerImage;
cv::Mat combinedImage;
cv::Mat selectedColor;
cv::Mat newBackgroundImage;
int textPixelHeight = 20;
int fontFace = cv::FONT_HERSHEY_SIMPLEX;
cv::Scalar fontColor = cv::Scalar( 0, 255, 0 );
int fontThickness = 2;
cv::Scalar backgroundColorBgr = cv::Scalar::all( 0 );
cv::Scalar backgroundColorHsv = cv::Scalar::all( 0 );
std::vector< std::string > helpStrings;
std::vector< std::string > headerStrings;
int colorTolerance { 15 };
int maxColorTolerance = { 50 };
int softness { 0 };
int maxSoftness = { 15 };
int colorCast { 0 };
int maxColorCast = { 100 };

void updateView( );
void showErrorMessage( const std::string& message );
void readNextFrame( cv::VideoCapture& cap );
void onMouse( int action, int x, int y, int flags, void* userdata );
cv::Scalar bgr2Hsv( const cv::Scalar& bgr );
void applyMattening( int, void* );
void scaleImageMax( const cv::Mat& imageIn, cv::Mat& imageOut );

int main( int argc, char** argv )
{
    std::ignore = argc;
    std::ignore = argv;

    // Initialize help and header strings
    headerStrings.push_back( "Patch Selector" );
    headerStrings.push_back( "Corrected Frame" );

    helpStrings.push_back(
        "Press left mouse in patch selector image to select background color" );
    helpStrings.push_back( "Press 'SPACE' for next frame" );
    helpStrings.push_back( "Press 'c' for continuous frame" );
    helpStrings.push_back( "Press 's' for single frame" );
    helpStrings.push_back( "Press 'ESC' for exit" );
    helpStrings.push_back( "" );
    helpStrings.push_back( "Selected Color" );

    newBackgroundImage =
        cv::imread( IMAGES_ROOT + "/background.jpg", cv::IMREAD_COLOR );

    // Initialize initial images
    currentFrame = cv::Mat( 720, 1280, CV_8UC3, cv::Scalar::all( 0 ) );
    resultFrame = currentFrame.clone( );

    int key { };

    cv::namedWindow( windowName );

    updateView( );

    // highgui function called when mouse events occur
    cv::setMouseCallback( windowName, onMouse );

    // Create Trackbars for the tolerance slider
    cv::createTrackbar( "Color Tolerance",
                        windowName,
                        &colorTolerance,
                        maxColorTolerance,
                        applyMattening );

    // Create Trackbars for the softness slider
    cv::createTrackbar(
        "Softness", windowName, &softness, maxSoftness, applyMattening );

    // Create Trackbars for the colro cats slider
    cv::createTrackbar(
        "Color Cast", windowName, &colorCast, maxColorCast, applyMattening );

    cv::VideoCapture videoCap( IMAGES_ROOT + "/greenscreen-asteroid.mp4" );

    // Check if stream opened successfully
    if ( ! videoCap.isOpened( ) )
    {
        showErrorMessage( "Error opening video stream or file" );
    }

    readNextFrame( videoCap );

    bool continuousMode { false };

    while ( key != 27 )
    {
        if ( continuousMode )
        {
            readNextFrame( videoCap );
        }

        if ( key == 'c' )
        {
            continuousMode = true;
        }

        if ( key == 's' )
        {
            continuousMode = false;
        }

        if ( key == 'a' )
        {
            applyMattening( 0, nullptr );
        }

        if ( key == 32 && ! continuousMode )
        {
            readNextFrame( videoCap );
        }

        key = cv::waitKey( 20 ) & 0xFF;
    }

    // Clean up
    videoCap.release( );
    cv::destroyAllWindows( );

    return 0;
}

void updateView( )
{
    cv::hconcat( currentFrame, resultFrame, combinedImage );

    if ( helpImage.size( ).width == 0 || helpImage.size( ).height == 0 )
    {
        const double fontScale = cv::getFontScaleFromHeight(
            fontFace, textPixelHeight, fontThickness );

        int helpImageHeight = textPixelHeight / 2;

        for ( const auto& text : helpStrings )
        {
            std::ignore = text;
            helpImageHeight += textPixelHeight;
            helpImageHeight += textPixelHeight / 2;
        }

        helpImageHeight += textPixelHeight / 2;

        helpImage = cv::Mat( helpImageHeight,
                             combinedImage.size( ).width,
                             CV_8UC3,
                             cv::Scalar::all( 0 ) );

        int textPosY = textPixelHeight + textPixelHeight / 2;
        const int textPosX = textPixelHeight;

        for ( auto& text : helpStrings )
        {
            putText( helpImage,
                     text,
                     cv::Point( textPosX, textPosY ),
                     fontFace,
                     fontScale,
                     fontColor,
                     fontThickness,
                     cv::LINE_AA );

            textPosY += textPixelHeight + textPixelHeight / 2;
        }
    }

    if ( headerImage.size( ).width == 0 || headerImage.size( ).height == 0 )
    {
        const int headerImageHeight = 2 * textPixelHeight;

        headerImage = cv::Mat( headerImageHeight,
                               combinedImage.size( ).width,
                               CV_8UC3,
                               cv::Scalar::all( 0 ) );

        int xOffset = textPixelHeight;
        const int yPos = textPixelHeight + textPixelHeight / 2;

        const double fontScale = cv::getFontScaleFromHeight(
            fontFace, textPixelHeight, fontThickness );

        for ( auto& text : headerStrings )
        {
            putText( headerImage,
                     text,
                     cv::Point( xOffset, yPos ),
                     fontFace,
                     fontScale,
                     fontColor,
                     fontThickness,
                     cv::LINE_AA );

            xOffset += currentFrame.size( ).width;
        }
    }

    if ( selectedColor.size( ).width == 0 || selectedColor.size( ).height == 0 )
    {
        selectedColor = cv::Mat(
            50, combinedImage.size( ).width, CV_8UC3, cv::Scalar::all( 0 ) );
    }
    else
    {
        selectedColor.setTo( backgroundColorBgr );
    }

    cv::vconcat( headerImage, combinedImage, combinedImage );

    cv::vconcat( combinedImage, helpImage, combinedImage );

    cv::vconcat( combinedImage, selectedColor, combinedImage );

    cv::imshow( windowName, combinedImage );
}

void showErrorMessage( const std::string& message )
{
    std::cout << message << '\n';

    const double fontScale =
        cv::getFontScaleFromHeight( fontFace, textPixelHeight, fontThickness );

    int posY = combinedImage.size( ).height / 2 - textPixelHeight;
    int posX = textPixelHeight;

    putText( combinedImage,
             message,
             cv::Point( posX, posY ),
             fontFace,
             fontScale,
             cv::Scalar( 0, 0, 255 ),
             fontThickness,
             cv::LINE_AA );

    cv::imshow( windowName, combinedImage );
}

void readNextFrame( cv::VideoCapture& cap )
{
    // Capture frame-by-frame
    cap >> currentFrame;

    // If the frame is empty, reset the stream
    if ( currentFrame.empty( ) )
    {
        std::cout << "Video stream reached end. Video restarted\n";
        cap.set( cv::CAP_PROP_POS_FRAMES, 0.0 );
        cap >> currentFrame;
    }

    if ( backgroundColorBgr != cv::Scalar::all( 0 ) )
    {
        applyMattening( 0, nullptr );
    }

    updateView( );
}

void onMouse( int action, int x, int y, int flags, void* userdata )
{
    std::ignore = action;
    std::ignore = x;
    std::ignore = y;
    std::ignore = flags;
    std::ignore = userdata;

    if ( action == cv::EVENT_LBUTTONDOWN )
    {
        // We have to correct the x and y coordinated, since we show help
        // information and other help in one frame where the mouse event is
        // captured.
        const int yOffset = headerImage.size( ).height;
        const int width = currentFrame.size( ).width;
        const int height = currentFrame.size( ).height;

        y -= yOffset;

        // Check if we are in a valid range
        if ( y < 0 || y >= height || x < 0 || x >= width )
        {
            std::cout << "Color patch selection out of valid range\n";
            return;
        }

        backgroundColorBgr = currentFrame.at< cv::Vec3b >( y, x );
        backgroundColorHsv = bgr2Hsv( backgroundColorBgr );

        updateView( );
    }
}

cv::Scalar bgr2Hsv( const cv::Scalar& bgr )
{
    auto isZero = []( double value )
    {
        return std::fabs( value ) <= std::numeric_limits< double >::epsilon( );
    };

    auto isEqual = []( double a, double b )
    {
        return std::fabs( a - b ) <= std::numeric_limits< double >::epsilon( );
    };

    const auto R = static_cast< double >( bgr[ 2 ] ) / 255.0;
    const auto G = static_cast< double >( bgr[ 1 ] ) / 255.0;
    const auto B = static_cast< double >( bgr[ 0 ] ) / 255.0;

    const auto Vmax = std::max( R, std::max( G, B ) );
    const auto Vmin = std::min( R, std::min( G, B ) );

    const auto VDelta = Vmax - Vmin;

    double S { };

    if ( ! isZero( Vmax ) )
    {
        S = VDelta / Vmax;
    }

    double H { };

    if ( isEqual( Vmax, R ) )
    {
        H = 60.0 * ( G - B ) / VDelta;
    }
    else if ( isEqual( Vmax, G ) )
    {
        H = 120.0 + 60.0 * ( B - R ) / VDelta;
    }
    else if ( isEqual( Vmax, B ) )
    {
        H = 240.0 + 60.0 * ( R - G ) / VDelta;
    }
    else if ( isZero( R ) || isZero( G ) || isZero( B ) )
    {
        H = 0.0;
    }

    if ( H < 0.0 )
    {
        H += 360.0;
    }

    H /= 2.0;

    return cv::Scalar( H, S * 255.0, Vmax * 255.0 );
}

void applyMattening( int, void* )
{
    int tgtWidth = currentFrame.size( ).width;
    int tgtHeight = currentFrame.size( ).height;

    cv::Mat bgToUse;
    cv::resize( newBackgroundImage, bgToUse, cv::Size( tgtWidth, tgtHeight ) );
    // showMat( bgToUse, "bgToUse" );

    auto lowerGreen = backgroundColorHsv;
    auto upperGreen = cv::Scalar::all( 255 );

    // Set bounds for the hue value
    int hueVal = static_cast< int >( lowerGreen[ 0 ] );

    if ( hueVal - colorTolerance >= 0 )
    {
        lowerGreen[ 0 ] = hueVal - colorTolerance;
    }
    else
    {
        lowerGreen[ 0 ] = 0;
    }

    if ( hueVal + colorTolerance <= 255 )
    {
        upperGreen[ 0 ] = hueVal + colorTolerance;
    }
    else
    {
        upperGreen = 255;
    }

    lowerGreen[ 1 ] = 50;
    lowerGreen[ 2 ] = 50;

    // Convert the current input image to HSV color space
    cv::Mat imgHsv;
    cv::cvtColor( currentFrame, imgHsv, cv::COLOR_BGR2HSV );

    // Create a mask of the HSC images with lower and upper bounds
    cv::Mat maskGreen;
    cv::inRange( imgHsv, lowerGreen, upperGreen, maskGreen );
    // showMat( maskGreen, "maskGreen" );

    // Invert the mask to get the green background as foreground
    cv::Mat maskGreenInv;
    cv::bitwise_not( maskGreen, maskGreenInv );
    // showMat( maskGreenInv, "maskGreenInv" );

    // If no softness is set, we can use bit mask operations to gain performance
    if ( softness == 0 )
    {
        cv::Mat currentFrameMasked;
        cv::Mat channels[ 3 ];
        channels[ 0 ] = maskGreenInv;
        channels[ 1 ] = maskGreenInv;
        channels[ 2 ] = maskGreenInv;
        cv::merge( channels, 3, maskGreenInv );
        cv::bitwise_and( currentFrame, maskGreenInv, currentFrameMasked );
        // showMat( currentFrameMasked, "currentFrameMasked" );

        cv::Mat backgroundMasked;
        channels[ 0 ] = maskGreen;
        channels[ 1 ] = maskGreen;
        channels[ 2 ] = maskGreen;
        cv::merge( channels, 3, maskGreen );
        cv::bitwise_and( bgToUse, maskGreen, backgroundMasked );
        // showMat( backgroundMasked, "backgroundMasked" );

        resultFrame = backgroundMasked + currentFrameMasked;
        // showMat( resultFrame, "resultFrame" );
    }
    else
    {
        int filterRadius = softness * 2 + 1;

        maskGreen.convertTo( maskGreen, CV_32F );
        maskGreen = maskGreen / 255.0;
        // showMat( maskGreen, "maskGreen" );

        maskGreenInv.convertTo( maskGreenInv, CV_32F );
        maskGreenInv = maskGreenInv / 255.0;
        // showMat( maskGreenInv, "maskGreenInv" );

        cv::Mat maskGreenBlurred;
        cv::GaussianBlur( maskGreen,
                          maskGreenBlurred,
                          cv::Size( filterRadius, filterRadius ),
                          0 );

        cv::Mat maskGreenInvBlurred = maskGreenInv.clone( );
        /*  cv::GaussianBlur( maskGreenInv,
                            maskGreenInvBlurred,
                            cv::Size( filterRadius, filterRadius ),
                            0 );*/

        cv::Mat curFrameFlt;
        currentFrame.convertTo( curFrameFlt, CV_32F );

        cv::Mat curBgFlt;
        bgToUse.convertTo( curBgFlt, CV_32F );

        cv::Mat channelsCurFrame[ 3 ];
        cv::Mat channelsBg[ 3 ];

        cv::split( curFrameFlt, channelsCurFrame );
        cv::split( curBgFlt, channelsBg );

        for ( int i = 0; i < 3; i++ )
        {
            cv::multiply( channelsCurFrame[ i ],
                          maskGreenInvBlurred,
                          channelsCurFrame[ i ] );

            cv::multiply( channelsBg[ i ], maskGreenBlurred, channelsBg[ i ] );
        }

        cv::Mat currentFrameMasked;
        cv::merge( channelsCurFrame, 3, currentFrameMasked );
        currentFrameMasked.convertTo( currentFrameMasked, CV_8UC3 );
        // showMat( currentFrameMasked, "currentFrameMasked" );

        cv::Mat backgroundMasked;
        cv::merge( channelsBg, 3, backgroundMasked );
        backgroundMasked.convertTo( backgroundMasked, CV_8UC3 );
        // showMat( backgroundMasked, "backgroundMasked" );

        cv::add( currentFrameMasked, backgroundMasked, resultFrame );

        // showMat( resultFrame, "resultFrame" );
    }

    if ( colorCast > 0 )
    {
        // https://stackoverflow.com/questions/70876252/how-to-do-color-cast-removal-or-color-coverage-in-python
        float castValue = static_cast< float >( colorCast ) / 100.0f;

        cv::Mat imageHSV;
        cv::cvtColor( resultFrame, imageHSV, cv::COLOR_BGR2HSV );

        // Separate the channels
        cv::Mat channels[ 3 ];
        cv::split( imgHsv, channels );

        // Get a clone of the hue channel to modify
        cv::Mat hueChannel = channels[ 0 ].clone( );

        // Shift the hue by 90 deg
        for ( int y = 0; y < hueChannel.rows; y++ )
        {
            const auto rowPtr = hueChannel.ptr< unsigned char >( y );
            for ( int x = 0; x < hueChannel.cols; x++ )
            {
                int val = rowPtr[ x ];

                rowPtr[ x ] =
                    static_cast< unsigned char >( ( val + 90 ) % 180 );
            }
        }

        // Assign the modified channel
        channels[ 0 ] = hueChannel;

        // Merge back to 3 channel image
        cv::merge( channels, 3, imgHsv );

        // Convert back to bgr
        cv::Mat bgrNew;
        cv::cvtColor( imgHsv, bgrNew, cv::COLOR_HSV2BGR );

        // Get the mean
        cv::Scalar mean;
        cv::Scalar stdDev;
        cv::meanStdDev( bgrNew, mean, stdDev );

        // Create a new image with the average color
        cv::Mat colorImage( resultFrame.size( ), CV_8UC3, mean );

        cv::Mat result;
        // Blend based on the input
        cv::addWeighted(
            resultFrame, 1.0f - castValue, colorImage, castValue, 0, result );

        scaleImageMax( result, resultFrame );
    }

    updateView( );
}

// Expected input is a color image
void scaleImageMax( const cv::Mat& imageIn, cv::Mat& imageOut )
{
    if ( imageOut.size( ) != imageIn.size( ) )
    {
        imageOut = imageIn.clone( );
    }

    cv::Mat channelsIn[ 3 ];
    cv::split( imageIn, channelsIn );

    cv::Mat channelsOut[ 3 ];
    cv::split( imageOut, channelsOut );

    const int imageWidth = imageIn.cols;
    const int imageHeight = imageIn.rows;

    for ( size_t i = 0; i < 3; i++ )
    {
        cv::Point minPos;
        cv::Point maxPos;
        double minGray { };
        double maxGray { };
        cv::minMaxLoc( channelsIn[ i ], &minGray, &maxGray, &minPos, &maxPos );

        double c1 = minGray * -1.0;
        double c2 = 255.0 / ( maxGray - minGray );

        for ( int32_t y = 0; y < imageHeight; y++ )
        {
            const auto ptrSrc = channelsIn[ i ].ptr( y );
            const auto ptrDst = channelsOut[ i ].ptr( y );

            for ( int32_t x = 0; x < imageWidth; x++ )
            {
                const auto newVal = static_cast< uint8_t >(
                    ( static_cast< double >( ptrSrc[ x ] ) + c1 ) * c2 );

                ptrDst[ x ] = newVal;
            }
        }
    }

    cv::merge( channelsOut, 3, imageOut );
}