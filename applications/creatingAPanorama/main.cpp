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
    // Step 1 : Find Keypoints and Descriptors
    //

    int MAX_FEATURES = 500;
    float GOOD_MATCH_PERCENT = 0.15f;

    // Read reference image
    const std::string image1File( IMAGES_ROOT + "/scene/scene1.jpg" );
    std::cout << "Reading First Image : " << image1File << '\n';
    cv::Mat im1 = cv::imread( image1File );

    const std::string image2File( IMAGES_ROOT + "/scene/scene3.jpg" );
    std::cout << "Reading Second Image : " << image2File << '\n';
    cv::Mat im2 = cv::imread( image2File );

    // Convert images to grayscale
    cv::Mat im1Gray, im2Gray;
    cv::cvtColor( im1, im1Gray, cv::COLOR_BGR2GRAY );
    cv::cvtColor( im2, im2Gray, cv::COLOR_BGR2GRAY );

    // Variables to store keypoints and descriptors
    std::vector< cv::KeyPoint > keypoints1, keypoints2;
    cv::Mat descriptors1, descriptors2;

    // Detect ORB features and compute descriptors.
    cv::Ptr< cv::Feature2D > orb = cv::ORB::create( MAX_FEATURES );
    orb->detectAndCompute( im1Gray, cv::Mat( ), keypoints1, descriptors1 );
    orb->detectAndCompute( im2Gray, cv::Mat( ), keypoints2, descriptors2 );

    cv::Mat im1Keypoints;
    cv::drawKeypoints( im1,
                       keypoints1,
                       im1Keypoints,
                       cv::Scalar( 0, 0, 255 ),
                       cv::DrawMatchesFlags::DEFAULT );
    std::cout << "Saving Image with Keypoint's\n";
    cv::imwrite( RESULTS_ROOT + "/keypoints.jpg", im1Keypoints );

    showMat( im1Keypoints, "Keypoint's obtained from the ORB detector", true );

    //
    // Step 2 : Find matching corresponding points
    //
    // Match features.
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
    cv::drawMatches( im1, keypoints1, im2, keypoints2, matches, imMatches );

    showMat( imMatches, "Matching obtained from the descriptor matcher", true );

    //
    // Step 3 : Image Alignment using Homography
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
    cv::Mat h = findHomography( points2, points1, cv::RANSAC );
    std::cout << "Homography Matrix:\n" << h << '\n';

    //
    // Step 4 : Warp Image
    //
    // Use homography to warp image
    //int im1Height = im1.rows;
    int im1Width = im1.cols;
    int im2Height = im2.rows;
    int im2Width = im2.cols;
    cv::Mat im2Aligned;
    cv::warpPerspective(
        im2, im2Aligned, h, cv::Size( im2Width + im1Width, im2Height ) );

    showMat( im2Aligned,
             "Second image aligned to first image obtained using homography "
             "and warping",
             true );

    //
    // Step 5 : Stitch Images
    //
    cv::Mat stitchedImage = im2Aligned.clone( );

    cv::Rect roi( 0, 0, im1.cols, im1.rows );
    im1.copyTo( stitchedImage( roi ) );

    showMat( stitchedImage,
             "Final Stitched Image",
             true );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}