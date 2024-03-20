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

// Fill the vector with random colors
void getRandomColors( std::vector< cv::Scalar >& colors, int numColors )
{
    cv::RNG rng( 0 );

    for ( int i = 0; i < numColors; i++ )
    {
        colors.emplace_back( rng.uniform( 0, 255 ),
                             rng.uniform( 0, 255 ),
                             rng.uniform( 0, 255 ) );
    }
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    const std::string videoFileName = IMAGES_ROOT + "/cycle.mp4";
    cv::VideoCapture cap( videoFileName );

    const auto width =
        static_cast< int32_t >( cap.get( cv::CAP_PROP_FRAME_WIDTH ) );
    const auto height =
        static_cast< int32_t >( cap.get( cv::CAP_PROP_FRAME_HEIGHT ) );

    cv::VideoWriter out( RESULTS_ROOT + "/sparse-output.mp4",
                         cv::VideoWriter::fourcc( 'M', 'P', '4', 'V' ),
                         20,
                         cv::Size( width, height ) );

    cv::TermCriteria termcrit(
        cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 10, 0.03 );

    // Take first frame and find corners in it
    cv::Mat old_frame;
    cap >> old_frame;

    showMat( old_frame, "First frame", true );

    cv::Mat old_gray;
    cvtColor( old_frame, old_gray, cv::COLOR_BGR2GRAY );

    std::vector< cv::Point2f > old_points;
    std::vector< uchar > status;
    std::vector< float > err;
    std::vector< cv::Point2f > new_points;

    std::vector< cv::Point2f > good_new;
    std::vector< cv::Point2f > good_old;

    std::vector< cv::Scalar > colors;

    cv::Point2f pt1, pt2;

    cv::goodFeaturesToTrack( old_gray,
                             old_points,
                             100,        // maxCorners
                             0.3,        // qualityLevel
                             7,          // minDistance
                             cv::Mat( ), // mask
                             7           // blockSize
    );

    cv::Mat display_frame;

    // Create a mask image for drawing the tracks
    cv::Mat mask = cv::Mat::zeros(
        old_frame.size( ).height, old_frame.size( ).width, CV_8UC3 );

    int count = 0;

    cv::Mat frame, frame_gray;

    while ( true )
    {
        cap >> frame;

        if ( frame.empty( ) )
        {
            std::cout << "over" << '\n';
        }

        cv::cvtColor( frame, frame_gray, cv::COLOR_BGR2GRAY );

        count += 1;

        // calculate optical flow
        cv::calcOpticalFlowPyrLK( old_gray,
                                  frame_gray,
                                  old_points,
                                  new_points,
                                  status,
                                  err,
                                  cv::Size( 15, 15 ), // winSize
                                  2,                  // maxLevel
                                  termcrit            // criteria
        );

        // Clearing the good points of the previous frames
        good_new.clear( );
        good_old.clear( );

        for ( size_t i = 0; i < new_points.size( ); i++ )
        {
            if ( status[ i ] == 1 )
            {
                good_new.push_back( new_points[ i ] );
                good_old.push_back( old_points[ i ] );
            }
        }

        getRandomColors( colors, static_cast< int >( new_points.size( ) ) );

        // draw the tracks
        for ( size_t j = 0; j < good_new.size( ); j++ )
        {
            pt1 = good_new[ j ];
            pt2 = good_old[ j ];
            cv::line( mask, pt1, pt2, colors[ j ], 2, cv::LINE_AA );
            cv::circle( frame, pt1, 3, colors[ j ], -1 );
        }

        cv::add( frame, mask, display_frame );

        out.write( display_frame );

        if ( count % 5 == 0 )
        {
            showMat( display_frame, "Frame " + std::to_string( count ), true );
        }

        if ( count > 50 )
        {
            break;
        }

        // Now update the previous frame and previous_points
        old_gray = frame_gray.clone( );
        old_points.clear( );
        old_points = good_new;
    }

    // Clean up
    cap.release( );
    out.release( );
    cv::destroyAllWindows( );

    return 0;
}