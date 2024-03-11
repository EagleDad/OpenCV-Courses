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

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    // Read image
    cv::Mat img = cv::imread( IMAGES_ROOT + "/lanes.jpg", cv::IMREAD_COLOR );
    // Convert the image to gray-scale
    cv::Mat gray;
    cv::cvtColor( img, gray, cv::COLOR_BGR2GRAY );
    // Find the edges in the image using canny detector
    cv::Mat edges;
    cv::Canny( gray, edges, 50, 200 );
    // Detect points that form a line
    std::vector< cv::Vec4i > lines;
    cv::HoughLinesP( edges, lines, 1, CV_PI / 180, 100, 10, 250 );

    // Draw lines on the image
    for ( size_t i = 0; i < lines.size( ); i++ )
    {
        cv::Vec4i line = lines[ i ];
        cv::line( img,
                  cv::Point( line[ 0 ], line[ 1 ] ),
                  cv::Point( line[ 2 ], line[ 3 ] ),
                  cv::Scalar( 255, 0, 0 ),
                  3,
                  cv::LINE_AA );
    }

    showMat( img, "Hough Lines", true );

    // Read image
    img = cv::imread( IMAGES_ROOT + "/circles.jpg", cv::IMREAD_COLOR );
    // Convert to gray-scale
    cv::cvtColor( img, gray, cv::COLOR_BGR2GRAY );

    // Blur the image to reduce noise
    cv::Mat img_blur;
    medianBlur( gray, img_blur, 5 );
    // Apply hough transform on the image
    std::vector< cv::Vec3f > circles;
    cv::HoughCircles( img_blur, circles, cv::HOUGH_GRADIENT, 1, 50, 450, 10, 30, 40 );

    for ( size_t i = 0; i < circles.size( ); i++ )
    {
        cv::Point center( cvRound( circles[ i ][ 0 ] ),
                          cvRound( circles[ i ][ 1 ] ) );
        int radius = cvRound( circles[ i ][ 2 ] );
        // Draw the outer circle
        cv::circle( img, center, radius, cv::Scalar( 0, 255, 0 ), 2 );
        // Draw the center of the circle
        cv::circle( img, center, 2, cv::Scalar( 0, 0, 255 ), 3 );
    }

    showMat( img, "Hough Circles", true );

    cv::destroyAllWindows( );

    return 0;
}