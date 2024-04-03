#include <GUI.h>
#include <macros.h>

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
IGNORE_WARNINGS_POP

// STD includes
#include <fstream>

/*
const std::string caffeConfigFile = MODEL_PATH + "deploy.prototxt";
const std::string caffeWeightFile = MODEL_PATH +
"res10_300x300_ssd_iter_140000_fp16.caffemodel";

const std::string tensorflowConfigFile = MODEL_PATH +
"opencv_face_detector.pbtxt"; const std::string tensorflowWeightFile =
MODEL_PATH + "opencv_face_detector_uint8.pb";

string filename = DATA_PATH + "images/panda.jpg";

 */

//
// Global variables
//
std::vector< std::string > help;
std::vector< std::string > headerTitle;
cv::Mat sourceImage;
cv::Mat resultImage;
cv::Mat overlayImage;
cv::Mat combinedImage;
cv::Mat helpImage;
cv::Mat headerImage;
int textPixelHeight = 20;
int fontFace = cv::FONT_HERSHEY_SIMPLEX;
cv::Scalar fontColor = cv::Scalar( 0, 255, 0 );
int fontThickness = 2;
std::string windowName = "Instagram Filters";
int blurRadius = 1;
int blurRadiusMax = 15;
int skinDetector = 1;
int skinDetectorMax = 1;
cv::dnn::Net net;

//
// Function declarations
//
std::vector< cv::Rect > detectFaces( const cv::Mat& image );
void updateView( const cv::Mat& result );
void skinSmoothing( );
void reset( );
void applySkinSmoothing( int, void* );
cv::Mat calculateMask( const cv::Rect& faceLocation );
cv::Mat zeroSumGameTheoryModelSkinMask( const cv::Rect& faceLocation );
cv::Mat meanColorSkinMask( const cv::Rect& faceLocation );

//
//
//
const std::string IMAGES_ROOT = "C:/images";
const std::string MODELS_ROOT = "C:/images/models";
const std::string RESULTS_ROOT = "C:/images/results";

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

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
#ifdef CAFFE
    net = cv::dnn::readNetFromCaffe( caffeConfigFile, caffeWeightFile );
#else
    net = cv::dnn::readNetFromTensorflow( tensorflowWeightFile,
                                          tensorflowConfigFile );
#endif

    help.emplace_back( "Press 's' for skin smoothing" );
    help.emplace_back( "Press 'r' for reset" );
    help.emplace_back( "Press 'ESC' for exit" );
    help.emplace_back( "" );
    help.emplace_back(
        "Skin detector: 0 -> Mean Value | 1 -> Zero-sum game theory model" );

    headerTitle.emplace_back( "Original Image" );
    headerTitle.emplace_back( "Overlay Image showing regions" );
    headerTitle.emplace_back( "Result Image" );

    //
    // Read the input image
    //
    // Consider multiple images: obama.jpg, first-image.jpg, family.jpg,
    // girl.jpg
    sourceImage = cv::imread( IMAGES_ROOT + "/hillary_clinton.jpg" );

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

    // Create a Trackbar for the blur radius
    cv::createTrackbar(
        "Blur", windowName, &blurRadius, blurRadiusMax, applySkinSmoothing );

    // Create a Trackbar for the skin detector
    cv::createTrackbar( "Skin",
                        windowName,
                        &skinDetector,
                        skinDetectorMax,
                        applySkinSmoothing );

    updateView( resultImage );

    while ( key != 27 )
    {
        if ( key == 's' )
        {
            resultImage = sourceImage.clone( );
            overlayImage = sourceImage.clone( );
            skinSmoothing( );
        }

        if ( key == 'r' )
        {
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

void skinSmoothing( )
{
    reset( );

    //
    // Detect face locations
    //
    const auto faceLocations = detectFaces( sourceImage );

    //
    // Perform skin detection
    //

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

        // Get the mask for the skin
        auto mask = calculateMask( loc );

        //
        // Show mask as overlay
        //
        cv::Mat maskOverlay;
        cv::Mat empty =
            cv::Mat( mask.rows, mask.cols, CV_8UC1, cv::Scalar( 0 ) );
        cv::merge( std::vector< cv::Mat > { mask, empty, empty }, maskOverlay );
        cv::Mat overlayRoi = overlayImage( loc );

        cv::addWeighted( overlayRoi, 0.75, maskOverlay, 0.5, 0, overlayRoi );


        //
        // Get the current roi
        //
        cv::Mat roi = sourceImage( loc ).clone( );

        // Scale the mask values to 0..1
        mask.convertTo( mask, CV_32F, 1 / 255.0f );

        // Convert the image to float values in range 0..1
        cv::Mat roiF;
        roi.convertTo( roiF, CV_32F, 1.0 / 255.0f );

        // Smooth the mask
        cv::GaussianBlur( mask, mask, cv::Size( 7, 7 ), 0, 0 );

        //
        // Perform filtering
        //

        // Blur the image using the edge-preserving filter
        cv::Mat imageBlurred;
        // cv::bilateralFilter( roiF, imageBlurred, 2 * 3, 30, 30 );
        cv::GaussianBlur( roiF,
                          imageBlurred,
                          cv::Size( blurRadius * 2 + 1, blurRadius * 2 + 1 ),
                          0 );

        // Combine the blurred and the original part of the image, making a
        // seamless transition between these regions

        // 1) The blurred part
        cv::Mat mask3F;
        cv::merge( std::vector< cv::Mat > { mask, mask, mask }, mask3F );
        cv::multiply( imageBlurred, mask3F, imageBlurred );

        // 2) The original part
        mask3F.convertTo( mask3F, -1, -1, 1 ); // invert the mask
        cv::multiply( roiF, mask3F, roiF );

        // 3) Combined image
        imageBlurred += roiF;

        imageBlurred.convertTo( imageBlurred, CV_8U, 255.0f );

        imageBlurred.copyTo( roi );

        // Get the result roi
        cv::Mat dstRoi = resultImage( loc );
        imageBlurred.copyTo( dstRoi );
    }

    updateView( resultImage );
}

void reset( )
{
    resultImage = sourceImage.clone( );
    overlayImage = resultImage.clone( );
    updateView( resultImage );
}

void applySkinSmoothing( int, void* )
{
    skinSmoothing( );
}

cv::Mat calculateMask( const cv::Rect& faceLocation )
{
    std::ignore = faceLocation;

    switch ( skinDetector )
    {
    case 0:
    {
        return meanColorSkinMask( faceLocation );
    }

    case 1:
    {
        return zeroSumGameTheoryModelSkinMask( faceLocation );
    }

    default:
        return { };
    }
}

cv::Mat zeroSumGameTheoryModelSkinMask( const cv::Rect& faceLocation )
{
    /*
    Djamila Dahmani, Mehdi Cheref, Slimane Larabi, Zero-sum game theory model
    for segmenting skin regions, Image and Vision Computing, Volume 99, 2020,
    103925,ISSN 0262-8856, https://doi.org/10.1016/j.imavis.2020.103925.
    */

    //
    // Get the current roi
    //
    cv::Mat roi = sourceImage( faceLocation );

    //
    // Convert input image to HSV and YCrCb
    //
    cv::Mat imageHsv;
    cv::Mat imageYCrCb;

    cv::cvtColor( roi, imageHsv, cv::COLOR_BGR2HSV );
    cv::cvtColor( roi, imageYCrCb, cv::COLOR_BGR2YCrCb );

    //
    // Get the mask in HSV range
    // The author proposed: 0<=H<=17 and 15<=S<=170 and 0<=V<=255
    //
    cv::Mat maskHsv;
    cv::inRange(
        imageHsv, cv::Scalar( 0, 15, 0 ), cv::Scalar( 17, 170, 255 ), maskHsv );

    //
    // Get the mask in YCrCb range
    // The author proposed: 0<=Y<=255 and 135<=Cr<=180 and 85<=Cb<=135
    //
    cv::Mat maskYCrCb;
    cv::inRange( imageYCrCb,
                 cv::Scalar( 0, 135, 85 ),
                 cv::Scalar( 255, 180, 135 ),
                 maskYCrCb );

    //
    // Remove some artifacts using morphology
    //
    const cv::Mat element =
        cv::getStructuringElement( cv::MORPH_RECT, cv::Size( 3, 3 ) );

    cv::morphologyEx( maskHsv, maskHsv, cv::MORPH_OPEN, element );
    cv::morphologyEx( maskYCrCb, maskYCrCb, cv::MORPH_OPEN, element );

    //
    // Merge skin masks
    //
    cv::Mat finaleMask;
    cv::bitwise_and( maskHsv, maskYCrCb, finaleMask );
    cv::morphologyEx( finaleMask, finaleMask, cv::MORPH_OPEN, element );

    return finaleMask;
}

cv::Mat meanColorSkinMask( const cv::Rect& faceLocation )
{
    //
    // Get the current roi
    //
    cv::Mat roi = sourceImage( faceLocation );

    //
    // Convert input image to HSV and YCrCb
    //
    cv::Mat imageHsv;

    cv::cvtColor( roi, imageHsv, cv::COLOR_BGR2HSV );

    cv::Scalar mean, stdDev;
    cv::meanStdDev( imageHsv, mean, stdDev );

    cv::Scalar lowerBound(
        mean[ 0 ] - stdDev[ 0 ], mean[ 1 ] - stdDev[ 1 ], 0 );

    cv::Scalar upperBound(
        mean[ 0 ] + stdDev[ 0 ], mean[ 1 ] + stdDev[ 1 ], 255 );

    cv::Mat mask;
    cv::inRange( imageHsv, lowerBound, upperBound, mask );

    constexpr double maxHue = 360;

    if ( lowerBound[ 0 ] < 0 && upperBound[ 0 ] < maxHue )
    {
        cv::Mat tmpMask;
        lowerBound[ 0 ] = lowerBound[ 0 ] + maxHue;
        upperBound[ 0 ] = maxHue;
        cv::inRange( imageHsv, lowerBound, upperBound, tmpMask );
        cv::bitwise_or( mask, tmpMask, mask );
    }
    else if ( lowerBound[ 0 ] > 0 && upperBound[ 0 ] > maxHue )
    {
        cv::Mat tmpMask;
        lowerBound[ 0 ] = 0;
        upperBound[ 0 ] = upperBound[ 0 ] - maxHue;
        cv::inRange( imageHsv, lowerBound, upperBound, tmpMask );
        cv::bitwise_or( mask, tmpMask, mask );
    }

    return mask;
}