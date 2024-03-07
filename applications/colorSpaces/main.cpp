#include <GUI.h>
#include <macros.h>

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
IGNORE_WARNINGS_POP

// STD includes
#include <iostream>

const std::string IMAGES_ROOT = "C:/images";
const std::string RESULTS_ROOT = "C:/images/results";

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    //
    // RGB Color Space
    //

    std::string imageName = IMAGES_ROOT + "/capsicum.jpg";

    // Read the input image
    cv::Mat bgr = cv::imread( imageName, cv::IMREAD_COLOR );

    showMat( bgr, "Input image", false );

    cv::Mat bgrChannels[ 3 ];
    cv::split( bgr, bgrChannels );

    showMat( bgrChannels[ 0 ], "Blue Channel", false );
    showMat( bgrChannels[ 1 ], "Green Channel", false );
    showMat( bgrChannels[ 2 ], "Red Channel", true );

    //
    // HSV Color Space
    //
    cv::Mat hsvImage;
    cvtColor( bgr, hsvImage, cv::COLOR_BGR2HSV );

    cv::Mat hsvChannels[ 3 ];
    cv::split( hsvImage, hsvChannels );

    showMat( hsvChannels[ 0 ], "Hue", false );
    showMat( hsvChannels[ 1 ], "Saturation", false );
    showMat( hsvChannels[ 2 ], "Value", true );

    // Value test
    cv::Mat imH, imS, imV, imhsv, imbgr;
    std::vector< cv::Mat > channels;

    for ( int i = 0; i < 7; i++ )
    {
        // Create 50x50 HSV channels
        // Set Hue = 0, Saturation = 0, Value = i x 40
        imH = cv::Mat::zeros( cv::Size( 50, 50 ), CV_8U );
        imS = cv::Mat::zeros( cv::Size( 50, 50 ), CV_8U );
        imV = cv::Mat::ones( cv::Size( 50, 50 ), CV_8U ) * ( i * 40 );

        channels = { imH, imS, imV };

        cv::merge( channels, imhsv );

        // Convert HSV to bgr
        cv::cvtColor( imhsv, imbgr, cv::COLOR_HSV2BGR );

        // Display image
        showMat( imbgr, "V=" + std::to_string( i * 40 ), false, 4 );
    }

    // Saturation Test
    for ( int i = 0; i < 7; i++ )
    {
        // Create 50x50 HSV channels
        // Set Hue = 0, Value = 128
        imH = cv::Mat::zeros( cv::Size( 50, 50 ), CV_8U );
        imS = cv::Mat::ones( cv::Size( 50, 50 ), CV_8U ) * ( i * 40 );
        imV = cv::Mat::ones( cv::Size( 50, 50 ), CV_8U ) * 128;

        channels = { imH, imS, imV };

        cv::merge( channels, imhsv );

        // Convert HSV to bgr
        cv::cvtColor( imhsv, imbgr, cv::COLOR_HSV2BGR );

        // Display image
        showMat( imbgr, "S=" + std::to_string( i * 40 ), false, 4 );
    }

    // Hue Test
    for ( int i = 0; i < 7; i++ )
    {
        // Create 50x50 HSV channels
        // Set Saturation = 128, Value = 128
        imH = cv::Mat::ones( cv::Size( 50, 50 ), CV_8U ) * ( i * 30 );
        imS = cv::Mat::ones( cv::Size( 50, 50 ), CV_8U ) * 128;
        imV = cv::Mat::ones( cv::Size( 50, 50 ), CV_8U ) * 128;

        channels = { imH, imS, imV };

        cv::merge( channels, imhsv );

        // Convert HSV to bgr
        cv::cvtColor( imhsv, imbgr, cv::COLOR_HSV2BGR );

        // Display image
        showMat( imbgr, "H=" + std::to_string( i * 30 ), false, 4 );
    }

    // The YCrCb color space
    // cv::Mat imH, imS, imV, imhsv, imbgr;
    // std::vector< cv::Mat > channels;

    for ( int i = 0; i < 7; i++ )
    {
        imH = cv::Mat::ones( cv::Size( 50, 50 ), CV_8U ) * ( i * 30 );
        imS = cv::Mat::ones( cv::Size( 50, 50 ), CV_8U ) * 128;
        imV = cv::Mat::ones( cv::Size( 50, 50 ), CV_8U ) * 128;

        channels = { imH, imS, imV };

        cv::merge( channels, imhsv );

        cv::cvtColor( imhsv, imbgr, cv::COLOR_HSV2BGR );
    }

    cv::Mat ycbImage;
    cv::cvtColor( bgr, ycbImage, cv::COLOR_BGR2YCrCb );
    cv::split( ycbImage, channels );

    showMat( channels[ 0 ], "Y Channel", false, 1 );
    showMat( channels[ 1 ], "Cr Channel", false, 1 );
    showMat( channels[ 2 ], "Cb Channel", true, 1 );

    // The Lab color space

    // convert from bgr to LAB format
    cv::Mat labImage;
    cv::cvtColor( bgr, labImage, cv::COLOR_BGR2Lab );

    cv::split( labImage, channels );

    showMat( channels[ 0 ], "L Channel", false, 1 );
    showMat( channels[ 1 ], "A Channel", false, 1 );
    showMat( channels[ 2 ], "B Channel", true, 1 );

    //
    // Application 1
    //
    //  Finding dominant color in an image
    std::string filename = IMAGES_ROOT + "/jersey.jpg";
    cv::Mat img = cv::imread( filename );

    showMat( img, "Messi Shirt", true, 1 );

    // Convert to HSV color space
    cv::cvtColor( img, hsvImage, cv::COLOR_BGR2HSV );

    // Split the channels
    cv::split( hsvImage, hsvChannels );
    cv::Mat H = hsvChannels[ 0 ], S = hsvChannels[ 1 ], V = hsvChannels[ 2 ];

    std::cout << H.size( ) << "\n";

    float range[] = { 0.f, 180.0f };

    int32_t levels = 255;
    const float* ranges[] = { range };
    constexpr int32_t hChannels[] = { 0 };
    const int32_t histSize[] = { levels };

    std::vector< float > histogram;

    cv::Mat matHistogram;
    cv::calcHist(
        &H, 1, hChannels, cv::noArray( ), matHistogram, 1, histSize, ranges );

    histogram.resize( static_cast< size_t >( levels ) );

    for ( int32_t row = 0; row < matHistogram.rows; row++ )
    {
        const auto histVal = matHistogram.at< float >( row, 0 );

        histogram[ static_cast< size_t >( row ) ] = histVal;
    }

    showBar( histogram, "H Histogram", true );

    //
    // Application 2
    //
    // Desaturation Filter
    filename = IMAGES_ROOT + "/girl.jpg";
    img = cv::imread( filename );

    showMat( img, "Girl", false, 1 );

    // Specify scaling factor
    float saturationScale = 0.01f;

    // Convert to HSV color space
    cv::cvtColor( img, hsvImage, cv::COLOR_BGR2HSV );

    // Convert to float32
    hsvImage.convertTo( hsvImage, CV_32F );

    // Split the channels
    cv::split( hsvImage, channels );

    // showMat( channels[ 0 ], "Girl Hue", false );
    // showMat( channels[ 1 ], "Girl Saturation", false );
    // showMat( channels[ 2 ], "Girl Value", true );
    cv::Mat imSat;
    for ( int i = 0; i < 200; i++ )
    {
        // Multiply S channel by scaling factor
        channels[ 1 ] = channels[ 1 ] * saturationScale;

        // Clipping operation performed to limit pixel values
        // between 0 and 255
        min( channels[ 1 ], 255, channels[ 1 ] );
        max( channels[ 1 ], 0, channels[ 1 ] );

        // Merge the channels
        merge( channels, hsvImage );

        // Convert back from float32
        hsvImage.convertTo( hsvImage, CV_8UC3 );

        
        // Convert to BGR color space
        cv::cvtColor( hsvImage, imSat, cv::COLOR_HSV2BGR );

        showMat( imSat, "Girl Saturated", false, 1 );

        cv::waitKey( 100 );

        saturationScale += 0.02f;

    }

    showMat( imSat, "Girl Saturated", true, 1 );

    cv::destroyAllWindows( );

    return 0;
}