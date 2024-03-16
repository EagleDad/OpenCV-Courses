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
    //
    // Step 1: Read in images
    //

    // Read reference image
    const std::string refFilename( IMAGES_ROOT + "/form.jpg" );
    std::cout << "Reading reference image : " << refFilename << '\n';
    const cv::Mat imReference = cv::imread( refFilename );

    // Read image to be aligned
    const std::string imFilename( IMAGES_ROOT + "/scanned-form.jpg" );
    std::cout << "Reading image to align : " << imFilename << '\n';
    cv::Mat img = cv::imread( imFilename );

    showMat( imReference, "Reference Image" );
    showMat( img, "Image to be aligned", true );

    //
    // Step 2: Detect features
    //
    int MAX_FEATURES = 500;
    float GOOD_MATCH_PERCENT = 0.15f;

    // Convert images to grayscale
    cv::Mat im1Gray, im2Gray;
    cv::cvtColor( img, im1Gray, cv::COLOR_BGR2GRAY );
    cv::cvtColor( imReference, im2Gray, cv::COLOR_BGR2GRAY );

    // Variables to store keypoints and descriptors
    std::vector< cv::KeyPoint > keypoints1, keypoints2;
    cv::Mat descriptors1, descriptors2;

    // Detect ORB features and compute descriptors.
    cv::Ptr< cv::Feature2D > orb = cv::ORB::create( MAX_FEATURES );
    orb->detectAndCompute( im1Gray, cv::Mat( ), keypoints1, descriptors1 );
    orb->detectAndCompute( im2Gray, cv::Mat( ), keypoints2, descriptors2 );

    cv::Mat imgKp1;
    cv::drawKeypoints( img,
                       keypoints1,
                       imgKp1,
                       cv::Scalar( 0, 0, 255 ),
                       cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

    cv::Mat imgKp2;
    cv::drawKeypoints( imReference,
                       keypoints2,
                       imgKp2,
                       cv::Scalar( 0, 0, 255 ),
                       cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

    showMat( imgKp2, "Reference Image KP" );
    showMat( imgKp1, "Image to be aligned KP", true );

    //
    // Step 3: Match features
    //
    std::vector< cv::DMatch > matches;
    cv::Ptr< cv::DescriptorMatcher > matcher =
        cv::DescriptorMatcher::create( "BruteForce-Hamming" );
    matcher->match( descriptors1, descriptors2, matches, cv::Mat( ) );

    // Sort matches by score
    std::sort( matches.begin( ), matches.end( ) );

    // Remove not so good matches
    const int numGoodMatches = static_cast< int >(
        static_cast< float >( matches.size( ) ) * GOOD_MATCH_PERCENT );
    matches.erase( matches.begin( ) + numGoodMatches, matches.end( ) );

    // Draw top matches
    cv::Mat imMatches;
    cv::drawMatches(
        img, keypoints1, imReference, keypoints2, matches, imMatches );
    cv::imwrite( RESULTS_ROOT + "/matches.jpg", imMatches );

    showMat( imMatches, "KP matches", true );

    //
    // Step 4: Calculate Homography
    //
    // Extract location of good matches
    std::vector< cv::Point2f > points1, points2;

    for ( size_t i = 0; i < matches.size( ); i++ )
    {
        points1.push_back(
            keypoints1[ static_cast< size_t >( matches[ i ].queryIdx ) ].pt );
        points2.push_back(
            keypoints2[ static_cast< size_t >( matches[ i ].trainIdx ) ].pt );
    }

    // Find homography
    cv::Mat h = findHomography( points1, points2, cv::RANSAC );

    //
    // Step 5:  Warping Image
    //
    // Use homography to warp image
    cv::Mat im1Reg;
    cv::warpPerspective( img, im1Reg, h, imReference.size( ) );

    showMat( im1Reg, "Image registered", true );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}