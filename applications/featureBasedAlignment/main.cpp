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
    // Step 1: Read Image
    //

    // Read 8-bit color image.
    // This is an image in which the three channels are
    // concatenated vertically.
    cv::Mat img = imread( IMAGES_ROOT + "/emir.jpg", cv::IMREAD_GRAYSCALE );

    // Find the width and height of the color image
    cv::Size sz = img.size( );
    int height = sz.height / 3;
    int width = sz.width;

    std::cout << "Image Size: " << sz << '\n';
    // Extract the three channels from the gray scale image
    std::vector< cv::Mat > channels;
    channels.push_back( img( cv::Rect( 0, 0, width, height ) ) );
    channels.push_back( img( cv::Rect( 0, height, width, height ) ) );
    channels.push_back( img( cv::Rect( 0, 2 * height, width, height ) ) );

    cv::Mat blue = channels[ 0 ];
    cv::Mat green = channels[ 1 ];
    cv::Mat red = channels[ 2 ];

    showMat( blue, "Emir Blue", false );
    showMat( green, "Emir Green", false );
    showMat( red, "Emir red", true );

    //
    // Step 2 : Detect Features - 6 Marks
    //

    ///
    /// YOUR CODE HERE
    ///
    int MAX_FEATURES = 5000;
    float GOOD_MATCH_PERCENT = 0.15f;

    // Initiate ORB detector
    cv::Ptr< cv::ORB > orb = cv::ORB::create( MAX_FEATURES );

    // find the keypoint's with ORB
    std::vector< cv::KeyPoint > keypointsBlue, keypointsGreen, keypointsRed;
    cv::Mat descriptorsBlue, descriptorsGreen, descriptorsRed;

    orb->detectAndCompute( blue, cv::Mat( ), keypointsBlue, descriptorsBlue );
    orb->detectAndCompute(
        green, cv::Mat( ), keypointsGreen, descriptorsGreen );
    orb->detectAndCompute( red, cv::Mat( ), keypointsRed, descriptorsRed );

    // START DEBUG
    cv::Mat imgKpBlue, imgKpGreen, imgKpRed;

    cv::drawKeypoints( blue,
                       keypointsBlue,
                       imgKpBlue,
                       cv::Scalar( 255, 0, 0 ),
                       cv::DrawMatchesFlags::DEFAULT );

    cv::drawKeypoints( green,
                       keypointsGreen,
                       imgKpGreen,
                       cv::Scalar( 0, 255, 0 ),
                       cv::DrawMatchesFlags::DEFAULT );

    cv::drawKeypoints( red,
                       keypointsRed,
                       imgKpRed,
                       cv::Scalar( 0, 0, 255 ),
                       cv::DrawMatchesFlags::DEFAULT );

    showMat( imgKpBlue, "Keypoint's Blue", false );
    showMat( imgKpGreen, "Keypoint's Green", false );
    showMat( imgKpRed, "Keypoint's Red", true );
    // END DEBUG

    //
    // Step 3: Match Features - 6 Marks
    //
    // Set up the matcher
    cv::Ptr< cv::DescriptorMatcher > matcher =
        cv::DescriptorMatcher::create( "BruteForce-Hamming" );

    // Find Matches or Corresponding points
    std::vector< cv::DMatch > matchesBlueGreen;
    matcher->match(
        descriptorsBlue, descriptorsGreen, matchesBlueGreen, cv::Mat( ) );

    // Sort matches by score
    std::sort( matchesBlueGreen.begin( ), matchesBlueGreen.end( ) );

    // Remove not so good matches
    int numGoodMatches = static_cast< int >(
        static_cast< float >( matchesBlueGreen.size( ) ) * GOOD_MATCH_PERCENT );
    matchesBlueGreen.erase( matchesBlueGreen.begin( ) + numGoodMatches,
                            matchesBlueGreen.end( ) );

    // Draw top matches
    cv::Mat imMatchesBlueGreen;
    cv::drawMatches( blue,
                     keypointsBlue,
                     green,
                     keypointsGreen,
                     matchesBlueGreen,
                     imMatchesBlueGreen );

    showMat( imMatchesBlueGreen, "Top matches blue green", true );

    //
    //
    std::vector< cv::DMatch > matchesRedGreen;

    matcher->match(
        descriptorsRed, descriptorsGreen, matchesRedGreen, cv::Mat( ) );

    // Sort matches by score
    std::sort( matchesRedGreen.begin( ), matchesRedGreen.end( ) );

    // Remove not so good matches
    numGoodMatches = static_cast< int >(
        static_cast< float >( matchesRedGreen.size( ) ) * GOOD_MATCH_PERCENT );
    matchesRedGreen.erase( matchesRedGreen.begin( ) + numGoodMatches,
                           matchesRedGreen.end( ) );

    // Draw top matches
    cv::Mat imMatchesRedGreen;
    drawMatches( red,
                 keypointsRed,
                 green,
                 keypointsGreen,
                 matchesRedGreen,
                 imMatchesRedGreen );

    showMat( imMatchesRedGreen, "Top matches red green", true );

    //
    // Step 4: Calculate Homography - 12 Marks
    //

    // Extract location of good matches
    ///
    /// YOUR CODE HERE
    ///

    // Blue Green

    std::vector< cv::Point2f > src_ptsBlueGreen, dst_ptsBlueGreen;

    for ( size_t i = 0; i < matchesBlueGreen.size( ); i++ )
    {
        src_ptsBlueGreen.push_back(
            keypointsBlue[ static_cast< size_t >(
                               matchesBlueGreen[ i ].queryIdx ) ]
                .pt );
        dst_ptsBlueGreen.push_back(
            keypointsGreen[ static_cast< size_t >(
                                matchesBlueGreen[ i ].trainIdx ) ]
                .pt );
    }

    cv::Mat hBlueGreen =
        findHomography( src_ptsBlueGreen, dst_ptsBlueGreen, cv::RANSAC );

    //
    // Red Green
    //

    std::vector< cv::Point2f > src_ptsRedGreen, dst_ptsRedGreen;

    for ( size_t i = 0; i < matchesRedGreen.size( ); i++ )
    {
        src_ptsRedGreen.push_back(
            keypointsRed[ static_cast< size_t >(
                              matchesRedGreen[ i ].queryIdx ) ]
                .pt );
        dst_ptsRedGreen.push_back(
            keypointsGreen[ static_cast< size_t >(
                                matchesRedGreen[ i ].trainIdx ) ]
                .pt );
    }

    cv::Mat hRedGreen =
        findHomography( src_ptsRedGreen, dst_ptsRedGreen, cv::RANSAC );

    //
    // Step 5: Warping Image - 6 Marks
    //

    // Use homography to find blueWarped and RedWarped images
    ///
    /// YOUR CODE HERE
    ///
    cv::Mat blueWarped, redWarped;
    cv::warpPerspective( blue, blueWarped, hBlueGreen, green.size( ) );
    cv::warpPerspective( red, redWarped, hRedGreen, green.size( ) );

    showMat( blueWarped, "Blue channel aligned w.r.t green channel", false );
    showMat( redWarped, "Red channel aligned w.r.t green channel", true );

    //
    // Step 6: Merge Channels
    //
    cv::Mat colorImage;
    std::vector< cv::Mat > colorImageChannels { blueWarped, green, redWarped };
    cv::merge( colorImageChannels, colorImage );

    cv::Mat originalImage;
    cv::merge( channels, originalImage );

    showMat( originalImage, "Original Mis-aligned Image", false );
    showMat( colorImage, "Aligned Image", true );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}