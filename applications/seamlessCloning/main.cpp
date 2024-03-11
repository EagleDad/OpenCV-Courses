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
    // Read images : src image will be cloned into dst
    cv::Mat src = cv::imread( IMAGES_ROOT + "/airplane.jpg" );
    cv::Mat dst = cv::imread( IMAGES_ROOT + "/sky.jpg" );

    showMat( src, "Source Image", false );
    showMat( dst, "Destination Image", false );

    // Create a rough mask around the airplane.
    cv::Mat src_mask = cv::Mat::zeros( src.rows, src.cols, src.depth( ) );

    // Define the mask as a closed polygon
    cv::Point poly[ 1 ][ 7 ];
    poly[ 0 ][ 0 ] = cv::Point( 4, 80 );
    poly[ 0 ][ 1 ] = cv::Point( 30, 54 );
    poly[ 0 ][ 2 ] = cv::Point( 151, 63 );
    poly[ 0 ][ 3 ] = cv::Point( 254, 37 );
    poly[ 0 ][ 4 ] = cv::Point( 298, 90 );
    poly[ 0 ][ 5 ] = cv::Point( 272, 134 );
    poly[ 0 ][ 6 ] = cv::Point( 43, 122 );

    const cv::Point* polygons[ 1 ] = { poly[ 0 ] };
    int num_points[] = { 7 };

    // Create mask by filling the polygon
    cv::fillPoly(
        src_mask, polygons, num_points, 1, cv::Scalar( 255, 255, 255 ) );

    showMat( src_mask, "Source Mask", false );

    // The location of the center of the src in the dst
    cv::Point center( 800, 100 );

    // Seamlessly clone src into dst and put the results in output
    cv::Mat normal_clone;
    cv::Mat mixed_clone;

    seamlessClone( src, dst, src_mask, center, normal_clone, cv::NORMAL_CLONE );
    seamlessClone( src, dst, src_mask, center, mixed_clone, cv::MIXED_CLONE );

    showMat( normal_clone, "Normal Clone", false );
    showMat( mixed_clone, "Mixed Clone", true );

    // Read images : src image will be cloned into dst
    cv::Mat img = cv::imread( IMAGES_ROOT + "/wood-texture.jpg" );
    cv::Mat obj = cv::imread( IMAGES_ROOT + "/iloveyouticket.jpg" );

    showMat( img, "Source Image", false );
    showMat( obj, "Destination Image", false );

    // Create an all white mask
    src_mask = 255 * cv::Mat::ones( obj.rows, obj.cols, obj.depth( ) );
    showMat( src_mask, "Source Mask", false );

    // The location of the center of the src in the dst
    center = cv::Point( img.cols / 2, img.rows / 2 );

    // Seamlessly clone src into dst and put the results in output
    seamlessClone( obj, img, src_mask, center, normal_clone, cv::NORMAL_CLONE );
    seamlessClone( obj, img, src_mask, center, mixed_clone, cv::MIXED_CLONE );

    showMat( normal_clone, "Normal Clone", false );
    showMat( mixed_clone, "Mixed Clone", true );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}