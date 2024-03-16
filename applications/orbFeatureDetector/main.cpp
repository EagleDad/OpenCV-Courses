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
    // Read input image
    cv::Mat img = cv::imread( IMAGES_ROOT + "/book.jpeg" );
    // Convert to grayscale
    cv::Mat imgGray;
    cvtColor( img, imgGray, cv::COLOR_BGR2GRAY );

    showMat( img, "Input Org", false );
    showMat( imgGray, "Input Gray", false );

    // Initiate ORB detector
    cv::Ptr< cv::ORB > orb = cv::ORB::create( );

    // find the keypoints with ORB
    std::vector< cv::KeyPoint > kp;
    orb->detect( imgGray, kp, cv::Mat( ) );

    // compute the descriptors with ORB
    cv::Mat des;
    orb->compute( imgGray, kp, des );

    // draw only keypoints location,not size and orientation
    cv::Mat img2;
    cv::drawKeypoints( img,
                   kp,
                   img2,
                   cv::Scalar( 0, 255, 0 ),
                   cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

    showMat( img2, "Input with Keypoint's", false );

    orb = cv::ORB::create( 10 );

    orb->detectAndCompute( imgGray, cv::Mat( ), kp, des );

    drawKeypoints( img,
                   kp,
                   img2,
                   cv::Scalar( 0, 0, 255 ),
                   cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

    // Size of the circle show teh scale of detection. Different checks after pydown
    // Line shows orientation

    showMat( img2, "Input with 10 Keypoint's", true );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}