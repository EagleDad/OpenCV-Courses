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
    // Read input images
    cv::Mat img1 = imread( IMAGES_ROOT + "/book.jpeg", cv::IMREAD_GRAYSCALE );
    cv::Mat img2 =
        imread( IMAGES_ROOT + "/book_scene.jpeg", cv::IMREAD_GRAYSCALE );

    showMat( img1, "Book", false );
    showMat( img2, "Book in a a scene", false );

    // Brute-Force Matching with ORB Descriptors

    // Initiate ORB detector
    cv::Ptr< cv::ORB > orb = cv::ORB::create( );

    // find the keypoints and descriptors with ORB
    std::vector< cv::KeyPoint > kp1, kp2;
    cv::Mat des1, des2;

    orb->detectAndCompute( img1, cv::Mat( ), kp1, des1 );
    orb->detectAndCompute( img2, cv::Mat( ), kp2, des2 );

    cv::Mat imgKp1;
    cv::drawKeypoints( img1,
                       kp1,
                       imgKp1,
                       cv::Scalar( 0, 255, 0 ),
                       cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

    cv::Mat imgKp2;
    cv::drawKeypoints( img2,
                       kp2,
                       imgKp2,
                       cv::Scalar( 0, 255, 0 ),
                       cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

    showMat( imgKp1, "KP Book", false );
    showMat( imgKp2, "KP Book in a a scene", false );

    // create BFMatcher object
    cv::BFMatcher bf( cv::NORM_HAMMING, true );

    // Match descriptors.
    std::vector< cv::DMatch > matches;
    bf.match( des1, des2, matches, cv::Mat( ) );

    // Sort them in the order of their distance
    std::sort( matches.begin( ), matches.end( ) );

    // Draw first 10 matches.
    matches = std::vector< cv::DMatch >( matches.begin( ), matches.begin( ) + 10 );
    cv::Mat img3;
    std::vector< char > match_mask( matches.size( ), 1 );
    cv::drawMatches( img1,
                 kp1,
                 img2,
                 kp2,
                 matches,
                 img3,
                 cv::Scalar::all( -1 ),
                 cv::Scalar::all( -1 ),
                 match_mask,
                 cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

    showMat( img3, "KP Book - Book scene BFM", false );

    // FLANN based Matcher
    cv::FlannBasedMatcher matcher( new cv::flann::LshIndexParams( 6, 12, 1 ),
                                   new cv::flann::SearchParams( 50 ) );

    std::vector< std::vector< cv::DMatch > > matches_vec;
    matcher.knnMatch( des1, des2, matches_vec, 2 );

    matches_vec = std::vector< std::vector< cv::DMatch > >( matches_vec.begin( ),
                                                            matches_vec.begin( ) + 10 );

    cv::drawMatches( img1, kp1, img2, kp2, matches_vec, img3 );

    showMat( img3, "KP Book - Book scene FLANN", true );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}