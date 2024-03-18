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
    int MAX_FEATURES = 1000;
    int MIN_MATCH_COUNT = 10;

    // Read both images
    cv::Mat img1 = cv::imread( IMAGES_ROOT + "/book.jpeg" );
    cv::Mat img1Gray;
    cv::cvtColor( img1, img1Gray, cv::COLOR_BGR2GRAY );

    cv::Mat img2 = cv::imread( IMAGES_ROOT + "/book_scene.jpeg" );
    cv::Mat img2Gray;
    cv::cvtColor( img2, img2Gray, cv::COLOR_BGR2GRAY );

    showMat( img1, "Input 1 Color", false );
    showMat( img1Gray, "Input 1 Gray", false );

    showMat( img2, "Input 2 Color", false );
    showMat( img2Gray, "Input 2 Gray", true );

    // Find Features in both images
    // Initiate ORB detector
    cv::Ptr< cv::ORB > orb = cv::ORB::create( MAX_FEATURES );

    // find the keypoints with ORB
    std::vector< cv::KeyPoint > keypoints1, keypoints2;
    cv::Mat descriptors1, descriptors2;

    orb->detectAndCompute( img1Gray, cv::Mat( ), keypoints1, descriptors1 );
    orb->detectAndCompute( img2Gray, cv::Mat( ), keypoints2, descriptors2 );

    // Set up the matcher
    cv::FlannBasedMatcher matcher( new cv::flann::KDTreeIndexParams( 5 ),
                                   new cv::flann::SearchParams( 50 ) );

    // Find Matches or Corresponding points
    std::vector< std::vector< cv::DMatch > > matches;
    descriptors1.convertTo( descriptors1, CV_32F );
    descriptors2.convertTo( descriptors2, CV_32F );
    matcher.knnMatch( descriptors1, descriptors2, matches, 2 );

    // Find good matches
    //-- Filter matches using the Lowe's ratio test
    const float ratio_thresh = 0.9f;
    std::vector< cv::DMatch > good;
    for ( size_t i = 0; i < matches.size( ); i++ )
    {
        if ( matches[ i ][ 0 ].distance <
             ratio_thresh * matches[ i ][ 1 ].distance )
        {
            good.push_back( matches[ i ][ 0 ] );
        }
    }

    std::vector< cv::Point2f > src_pts;
    std::vector< cv::Point2f > dst_pts;
    for ( size_t i = 0; i < good.size( ); i++ )
    {
        //-- Get the keypoints from the good matches
        src_pts.push_back(
            keypoints1[ static_cast< size_t >( good[ i ].queryIdx ) ].pt );
        dst_pts.push_back(
            keypoints2[ static_cast< size_t >( good[ i ].trainIdx ) ].pt );
    }

    // Find the location of the book in the cluttered image

    //-- Draw matches
    cv::Mat img3;
    drawMatches( img1,
                 keypoints1,
                 img2,
                 keypoints2,
                 good,
                 img3,
                 cv::Scalar( 0, 255, 0 ),
                 cv::Scalar::all( -1 ),
                 std::vector< char >( ),
                 cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

    if ( static_cast< int >( good.size( ) ) > MIN_MATCH_COUNT )
    {
        cv::Mat H = findHomography( src_pts, dst_pts, cv::RANSAC, 5.0 );

        //-- Get the corners from the image_1 ( the object to be "detected" )
        std::vector< cv::Point2f > obj_corners( 4 );
        obj_corners[ 0 ] = cv::Point2f( 0, 0 );
        obj_corners[ 1 ] = cv::Point2f( static_cast< float >( img1.cols ), 0 );
        obj_corners[ 2 ] = cv::Point2f( static_cast< float >( img1.cols ),
                                        static_cast< float >( img1.rows ) );
        obj_corners[ 3 ] = cv::Point2f( 0, static_cast< float >( img1.rows ) );
        std::vector< cv::Point2f > scene_corners( 4 );
        perspectiveTransform( obj_corners, scene_corners, H );

        //-- Draw lines between the corners (the mapped object in the scene -
        // image_2 )
        line( img3,
              scene_corners[ 0 ] +
                  cv::Point2f( static_cast< float >( img1.cols ), 0 ),
              scene_corners[ 1 ] +
                  cv::Point2f( static_cast< float >( img1.cols ), 0 ),
              cv::Scalar( 0, 0, 255 ),
              10 );
        line( img3,
              scene_corners[ 1 ] +
                  cv::Point2f( static_cast< float >( img1.cols ), 0 ),
              scene_corners[ 2 ] +
                  cv::Point2f( static_cast< float >( img1.cols ), 0 ),
              cv::Scalar( 0, 0, 255 ),
              10 );
        line( img3,
              scene_corners[ 2 ] +
                  cv::Point2f( static_cast< float >( img1.cols ), 0 ),
              scene_corners[ 3 ] +
                  cv::Point2f( static_cast< float >( img1.cols ), 0 ),
              cv::Scalar( 0, 0, 255 ),
              10 );
        line( img3,
              scene_corners[ 3 ] +
                  cv::Point2f( static_cast< float >( img1.cols ), 0 ),
              scene_corners[ 0 ] +
                  cv::Point2f( static_cast< float >( img1.cols ), 0 ),
              cv::Scalar( 0, 0, 255 ),
              10 );
    }
    else
    {
        std::cout << "Not enough matches are found - " << good.size( ) << "/"
                  << MIN_MATCH_COUNT << '\n';
    }

    showMat( img3, "Result of match", true );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}