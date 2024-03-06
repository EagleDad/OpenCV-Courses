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
    // Read image
    cv::Mat img =
        cv::imread( IMAGES_ROOT + "/blob_detection.jpg", cv::IMREAD_GRAYSCALE );

    // Set up detector with default parameters
    /*cv::Ptr< cv::SimpleBlobDetector > detector =
        cv::SimpleBlobDetector::create( );*/

    // Setup SimpleBlobDetector parameters.
    cv::SimpleBlobDetector::Params params;

    // Change thresholds
    params.minThreshold = 10;
    params.maxThreshold = 200;

    // Filter by Area.
    params.filterByArea = true;
    params.minArea = 1500;

    // Filter by Circularity
    params.filterByCircularity = true;
    params.minCircularity = 0.1f;

    // Filter by Convexity
    params.filterByConvexity = true;
    params.minConvexity = 0.87f;

    // Filter by Inertia
    params.filterByInertia = true;
    params.minInertiaRatio = 0.01f;

    const cv::Ptr< cv::SimpleBlobDetector > detector =
        cv::SimpleBlobDetector::create( /*params*/ );

    std::vector< cv::KeyPoint > keypoints;
    detector->detect( img, keypoints );

    // Mark blobs using image annotation concepts we have studied so far
    int x, y;
    int radius;
    double diameter;
    cv::cvtColor( img, img, cv::COLOR_GRAY2BGR );

    for ( int i = 0; i < static_cast< int >( keypoints.size( ) ); i++ )
    {
        cv::KeyPoint k = keypoints[ static_cast< size_t >( i ) ];
        cv::Point keyPt;
        keyPt = k.pt;
        x = static_cast< int >( keyPt.x );
        y = static_cast< int >( keyPt.y );
        // Mark center in BLACK
        circle( img, cv::Point( x, y ), 5, cv::Scalar( 255, 0, 0 ), -1 );
        // Get radius of coin
        diameter = k.size;
        radius = static_cast< int >( diameter / 2.0 );
        // Mark blob in GREEN
        cv::circle(
            img, cv::Point( x, y ), radius, cv::Scalar( 0, 255, 0 ), 2 );
    }

    showMat( img, "Original image", true );

    // Clean up
    system( "pause" ); // NOLINT(concurrency-mt-unsafe)
    cv::destroyAllWindows( );

    return 0;
}