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

// Variables for source and edge images
cv::Mat source, edges;
// Variables for low and high thresholds
int lowThreshold = 50;
int highThreshold = 100;

// Max trackbar value
int const maxThreshold = 1000;

// Sobel aperture sizes for Canny edge detector
int apertureSizes[] = { 3, 5, 7 };
int maxapertureIndex = 2;
int apertureIndex = 0;

// Gaussian blur size
int blurAmount = 0;
int maxBlurAmount = 20;

// Function for trackbar call
void applyCanny( int, void* )
{
    // Variable to store blurred image
    cv::Mat blurredSrc;

    // Blur the image before edge detection
    if ( blurAmount > 0 )
    {
        cv::GaussianBlur( source,
                          blurredSrc,
                          cv::Size( 2 * blurAmount + 1, 2 * blurAmount + 1 ),
                          0 );
    }
    else
    {
        blurredSrc = source.clone( );
    }

    // Canny requires aperture size to be odd
    int apertureSize = apertureSizes[ apertureIndex ];

    // Apply canny to get the edges
    cv::Canny( blurredSrc, edges, lowThreshold, highThreshold, apertureSize );

    cv::Mat matShow;
    cv::hconcat( source, edges, matShow );

    // Display images
    cv::imshow( "Edges", matShow );
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    // Read lena image
    //source = cv::imread( IMAGES_ROOT + "/sample.jpg", cv::IMREAD_GRAYSCALE );
    source = cv::imread( IMAGES_ROOT + "/Picture Vision-Ring_001.bmp", cv::IMREAD_GRAYSCALE );
    

    // Display images
    cv::imshow( "Edges", source );

    // Create a window to display output.
    cv::namedWindow( "Edges", cv::WINDOW_AUTOSIZE );

    // Trackbar to control the low threshold
    cv::createTrackbar(
        "Low Threshold", "Edges", &lowThreshold, maxThreshold, applyCanny );

    // Trackbar to control the high threshold
    cv::createTrackbar(
        "High Threshold", "Edges", &highThreshold, maxThreshold, applyCanny );

    // Trackbar to control the aperture size
    cv::createTrackbar( "aperture Size",
                        "Edges",
                        &apertureIndex,
                        maxapertureIndex,
                        applyCanny );

    // Trackbar to control the blur
    cv::createTrackbar(
        "Blur", "Edges", &blurAmount, maxBlurAmount, applyCanny );

    cv::waitKey( 0 );

    cv::destroyAllWindows( );

    return 0;
}