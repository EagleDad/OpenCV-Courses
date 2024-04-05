#include <GUI.h>
#include <macros.h>

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
IGNORE_WARNINGS_POP

const std::string IMAGES_ROOT = "C:/images";
const std::string MODELS_ROOT = "C:/images/models";
const std::string RESULTS_ROOT = "C:/images/results";

//
// Function declarations
//
std::vector< cv::Rect > detectFaces( const cv::Mat& image );
void updateView( const cv::Mat& result );
void reset( );
void applySunglassesFilter( int, void* );
void sunglassesFilter( );
cv::Rect getEyeRegion( const cv::Rect& faceLocation );

//
// Global CNN variables
//
cv::dnn::Net net;
constexpr size_t inWidth = 300;
constexpr size_t inHeight = 300;
constexpr double inScaleFactor = 1.0;
constexpr float confidenceThreshold = 0.6f;
const cv::Scalar meanVal( 104.0, 177.0, 123.0 );
const std::string caffeConfigFile = MODELS_ROOT + "/deploy.prototxt";
const std::string caffeWeightFile =
    MODELS_ROOT + "/res10_300x300_ssd_iter_140000_fp16.caffemodel";
const std::string tensorflowConfigFile =
    MODELS_ROOT + "/opencv_face_detector.pbtxt";
const std::string tensorflowWeightFile =
    MODELS_ROOT + "/opencv_face_detector_uint8.pb";

// #define CAFFE

//
// UI Interface
//
std::string windowName = "Instagram Filters";
std::vector< std::string > help;
std::vector< std::string > headerTitle;
int textPixelHeight = 20;
int fontFace = cv::FONT_HERSHEY_SIMPLEX;
cv::Scalar fontColor = cv::Scalar( 0, 255, 0 );
int fontThickness = 2;
int transparency = 75;
int transparencyMax = 100;
int reflection = 75;
int reflectionMax = 100;
char currentMode { };

//
// Global images
//
cv::Mat sourceImage;
cv::Mat resultImage;
cv::Mat overlayImage;
cv::Mat combinedImage;
cv::Mat helpImage;
cv::Mat headerImage;
cv::Mat sunglassesOrg;
cv::Mat reflectionOrg;
cv::Mat dropsOrg;

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    help.emplace_back( "Press 's' for sunglasses" );
    help.emplace_back( "Press 'm' for mountain reflections sunglasses" );
    help.emplace_back( "Press 'd' for drops on sunglasses" );
    help.emplace_back( "Press 'r' for reset" );
    help.emplace_back( "Press 'ESC' for exit" );
    help.emplace_back( "" );
    help.emplace_back( "Note: Transparency is applied to the glasses and reflectivity to mountains, not to the drops." );

    headerTitle.emplace_back( "Original Image" );
    headerTitle.emplace_back( "Overlay Image showing regions" );
    headerTitle.emplace_back( "Result Image" );

    //
    // Load the network
    //
#ifdef CAFFE
    net = cv::dnn::readNetFromCaffe( caffeConfigFile, caffeWeightFile );
#else
    net = cv::dnn::readNetFromTensorflow( tensorflowWeightFile,
                                          tensorflowConfigFile );
#endif

    //
    // Read the input images
    //
    sourceImage = cv::imread( IMAGES_ROOT + "/musk.jpg" );
    sunglassesOrg = cv::imread( IMAGES_ROOT + "/sunglass.png", -1 );
    reflectionOrg = cv::imread( IMAGES_ROOT + "/Mountains.png" );
    dropsOrg = cv::imread( IMAGES_ROOT + "/raindrops.jpg" );

    // Make sure that the image fits to a normal screen
    if ( sourceImage.cols > 512 )
    {
        const auto scale = 512.0 / sourceImage.cols;

        cv::resize( sourceImage, sourceImage, cv::Size( ), scale, scale );
    }

    int key { };
    cv::namedWindow( windowName );

    resultImage = sourceImage.clone( );
    overlayImage = sourceImage.clone( );

    // Create a Trackbar for the transparency
    cv::createTrackbar( "Transparency",
                        windowName,
                        &transparency,
                        transparencyMax,
                        applySunglassesFilter );

    // Create a Trackbar for the reflection
    cv::createTrackbar( "Reflection",
                        windowName,
                        &reflection,
                        reflectionMax,
                        applySunglassesFilter );

    updateView( resultImage );

    while ( key != 27 )
    {
        if ( key == 's' )
        {
            resultImage = sourceImage.clone( );
            overlayImage = sourceImage.clone( );
            currentMode = static_cast< char >( key );
            sunglassesFilter( );
        }

        if ( key == 'm' )
        {
            resultImage = sourceImage.clone( );
            overlayImage = sourceImage.clone( );
            currentMode = static_cast< char >( key );
            sunglassesFilter( );
        }

        if ( key == 'd' )
        {
            resultImage = sourceImage.clone( );
            overlayImage = sourceImage.clone( );
            currentMode = static_cast< char >( key );
            sunglassesFilter( );
        }

        if ( key == 'r' )
        {
            currentMode = static_cast< char >( key );
            reset( );
        }

        key = cv::waitKey( 20 ) & 0xFF;
    }

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}

std::vector< cv::Rect > detectFaces( const cv::Mat& image )
{
    std::vector< cv::Rect > faceLocations;

    const auto imageHeight = image.rows;
    const auto imageWidth = image.cols;

    const cv::Mat inputBlob =
        cv::dnn::blobFromImage( image,
                                inScaleFactor,
                                cv::Size( inWidth, inHeight ),
                                meanVal,
                                false,
                                false );

    net.setInput( inputBlob, "data" );

    cv::Mat detection = net.forward( "detection_out" );

    cv::Mat detectionMat( detection.size[ 2 ],
                          detection.size[ 3 ],
                          CV_32F,
                          detection.ptr< float >( ) );

    for ( int i = 0; i < detectionMat.rows; i++ )
    {
        const float confidence = detectionMat.at< float >( i, 2 );

        if ( confidence > confidenceThreshold )
        {
            const int x1 =
                static_cast< int >( detectionMat.at< float >( i, 3 ) *
                                    static_cast< float >( imageWidth ) );
            const int y1 =
                static_cast< int >( detectionMat.at< float >( i, 4 ) *
                                    static_cast< float >( imageHeight ) );
            const int x2 =
                static_cast< int >( detectionMat.at< float >( i, 5 ) *
                                    static_cast< float >( imageWidth ) );
            const int y2 =
                static_cast< int >( detectionMat.at< float >( i, 6 ) *
                                    static_cast< float >( imageHeight ) );

            if ( x2 < imageWidth && y2 < imageHeight && x1 >= 0 && y1 >= 0 )
            {
                faceLocations.emplace_back( cv::Point( x1, y1 ),
                                            cv::Point( x2, y2 ) );
            }
        }
    }

    return faceLocations;
}

void updateView( const cv::Mat& result )
{
    cv::hconcat( sourceImage, overlayImage, combinedImage );
    cv::hconcat( combinedImage, result, combinedImage );

    if ( helpImage.size( ).width == 0 || helpImage.size( ).height == 0 )
    {
        const double fontScale = cv::getFontScaleFromHeight(
            fontFace, textPixelHeight, fontThickness );

        int helpImageHeight = textPixelHeight / 2;

        for ( const auto& text : help )
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

        for ( auto& text : help )
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

        for ( auto& text : headerTitle )
        {
            putText( headerImage,
                     text,
                     cv::Point( xOffset, yPos ),
                     fontFace,
                     fontScale,
                     fontColor,
                     fontThickness,
                     cv::LINE_AA );

            xOffset += sourceImage.size( ).width;
        }
    }

    cv::vconcat( headerImage, combinedImage, combinedImage );

    cv::vconcat( combinedImage, helpImage, combinedImage );

    cv::imshow( windowName, combinedImage );
}

void reset( )
{
    resultImage = sourceImage.clone( );
    overlayImage = resultImage.clone( );
    updateView( resultImage );
}

void applySunglassesFilter( int, void* )
{
    sunglassesFilter( );
}

void sunglassesFilter( )
{
    //
    // Detect face locations
    //
    const auto faceLocations = detectFaces( sourceImage );

    for ( const auto& loc : faceLocations )
    {
        //
        // Show face locations
        //
        cv::rectangle( overlayImage,
                       loc,
                       cv::Scalar( 0, 255, 0 ),
                       sourceImage.rows / 150,
                       8 );

        //
        // Get the roi of the source image
        //
        cv::Mat roi = sourceImage( loc );

        //
        // Show eye region
        //
        const auto eyeRegion = getEyeRegion( loc );

        cv::rectangle( overlayImage,
                       eyeRegion,
                       cv::Scalar( 255, 0, 0 ),
                       sourceImage.rows / 150,
                       8 );

        //
        // Resize the sunglasses mask to fit to the face
        //
        cv::Mat sunglassesResized;
        cv::resize( sunglassesOrg,
                    sunglassesResized,
                    cv::Size( eyeRegion.width, eyeRegion.height ) );

        // Separate the Color and alpha channels
        cv::Mat glassRGBAChannels[ 4 ];
        cv::Mat glassRGBChannels[ 3 ];
        cv::split( sunglassesResized, glassRGBAChannels );

        for ( int i = 0; i < 3; i++ )
        {
            // Copy R,G,B channel from RGBA to RGB (shallow)
            glassRGBChannels[ i ] = glassRGBAChannels[ i ];
        }

        // Separate the channels of the source image
        cv::Mat sourceChannels[ 3 ];
        cv::split( roi, sourceChannels );

        cv::Mat glassBGR, glassMask1;
        // Prepare BGR Image
        cv::merge( glassRGBChannels, 3, glassBGR );
        // Alpha channel is the 4th channel in RGBA Image
        glassMask1 = glassRGBAChannels[ 3 ];

        cv::Mat fancyFilter;
        double reflectivity { };
        switch ( currentMode )
        {
        case 'm':
            // Adapt the size of the reflection image to the size of the
            // sunglasses
            cv::resize( reflectionOrg, fancyFilter, sunglassesResized.size( ) );
            reflectivity = reflection / 100.0;
            break;

        case 'd':
            // Adapt the size of the reflection image to the size of the
            // sunglasses
            reflectivity = 1.0;
            cv::resize( dropsOrg, fancyFilter, sunglassesResized.size( ) );
            break;

        default:
            break;
        }

        // Make the dimensions of the mask same as the input image.
        // Since Face Image is a 3-channel image, we create a 3 channel image
        // for the mask
        cv::Mat glassMask;
        cv::Mat glassMaskChannels[] = { glassMask1, glassMask1, glassMask1 };
        cv::merge( glassMaskChannels, 3, glassMask );

        // Get the eye region
        cv::Mat eyeRoi = sourceImage( eyeRegion );

        const auto alpha = transparency / 100.0;

        // Convert images to float
        cv::Mat eyeRoiF;
        eyeRoi.convertTo( eyeRoiF, CV_32F, 1.0 / 255.0 );

        cv::Mat maskAlphaF;
        glassMask.convertTo( maskAlphaF, CV_32F, 1.0 / 255.0 );

        cv::Mat glassesBgrF;
        glassBGR.convertTo( glassesBgrF, CV_32F, 1.0 / 255.0 );

        cv::Mat fancyFilterF;
        fancyFilter.convertTo( fancyFilterF, CV_32F, 1.0 / 255.0 );

        if ( fancyFilter.rows > 0 )
        {
            // Note: The reflectivity is only applied to the mountain image,
            // note to the drops

            // Multiply reflection image with reflectivity to make it semi
            // transparent
            cv::multiply(
                fancyFilterF, maskAlphaF, fancyFilterF, reflectivity );

            cv::multiply(
                glassesBgrF, maskAlphaF, glassesBgrF, 1.0 - reflectivity );

            glassesBgrF += fancyFilterF;
        }

        // Create the overlay
        // The alpha needs to be applied to the sunglasses and the inverse alpha
        // to the eyeRegion
        // SunglassesA = Sunglasses * mask * alpha
        // EyeRegionA = EyeRegion * (1 - mask * alpha) = EyeRegion - EyeRegion *
        // mask * alpha Overlay = EyeRegionA + SunglassesA = EyeRegion -
        // EyeRegion * mask * alpha + Sunglasses * mask * alpha = EyeRegion +
        // mask * alpha * (Sunglasses - EyeRegion)
        glassesBgrF -= eyeRoiF;
        cv::multiply( glassesBgrF, maskAlphaF, glassesBgrF, alpha );
        eyeRoiF += glassesBgrF;
        cv::Mat ouImage;
        eyeRoiF.convertTo( ouImage, CV_8U, 255 );

        // Copy the result to the output image
        cv::Mat dstRoi = resultImage( eyeRegion );
        ouImage.convertTo( ouImage, CV_8U );
        ouImage.copyTo( dstRoi );
    }

    updateView( resultImage );
}

cv::Rect getEyeRegion( const cv::Rect& faceLocation )
{
    // The typical proportion of the human head from the top of the head to the
    // chin can be separated into 4 quarters.
    // Top of the head to hairline 1/4
    // Hairline to center of eyes 1/4
    // Center of eyes to region between nose and mouth 1/4
    // region between nose and mouth to chin 1/4

    // The face detector will return the roi from the hairline to the chin.
    // This needs to be considered here

    const auto faceHeight = faceLocation.height;
    const auto quarter = static_cast< float >( faceHeight ) / 3.0f;

    const auto x1 = faceLocation.x;
    const auto x2 = faceLocation.x + faceLocation.width - 1;

    const auto y1 = faceLocation.y +
                    static_cast< int >( std::round( quarter ) ) -
                    static_cast< int >( std::round( quarter / 5.0f ) );

    const auto y2 = y1 + static_cast< int >( std::round( quarter / 1.5f ) );

    return cv::Rect( cv::Point( x1, y1 ), cv::Point( x2, y2 ) );
}