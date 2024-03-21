#include <GUI.h>
#include <macros.h>

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/tracking/tracking_legacy.hpp>
IGNORE_WARNINGS_POP

// STD includes
#include <iostream>

const std::string IMAGES_ROOT = "C:/images";
const std::string RESULTS_ROOT = "C:/images/results";

std::vector< std::string > trackerTypes = {
    "BOOSTING", "MIL", "KCF", "TLD", "MEDIANFLOW", "GOTURN", "CSRT", "MOSSE" };

// Fill the vector with random colors
void getRandomColors( std::vector< cv::Scalar >& colors, int numColors )
{
    cv::RNG rng( 0 );
    for ( int i = 0; i < numColors; i++ )
        colors.push_back( cv::Scalar( rng.uniform( 0, 255 ),
                                      rng.uniform( 0, 255 ),
                                      rng.uniform( 0, 255 ) ) );
}

// create tracker by name
auto createTrackerByName( std::string trackerType )
{
    cv::Ptr< cv::legacy::Tracker > tracker { nullptr };

    if ( trackerType == "BOOSTING" )
    {
        tracker = cv::legacy::TrackerBoosting::create( );
    }
    /* else if ( trackerType == "MIL" )
     {
         tracker = cv::TrackerMIL::create( );
     }*/
    // else if ( trackerType == "KCF" )
    //{
    //     tracker = cv::TrackerKCF::create( );
    // }
    else if ( trackerType == "TLD" )
    {
        tracker = cv::legacy::TrackerTLD::create( );
    }
    else if ( trackerType == "MEDIANFLOW" )
    {
        tracker = cv::legacy::TrackerMedianFlow::create( );
    }
    // else if ( trackerType == "GOTURN" )
    //{
    //     tracker = cv::TrackerGOTURN::create( );
    // }
    // else if ( trackerType == "CSRT" )
    //{
    //     tracker = cv::TrackerCSRT::create( );
    // }
    else if ( trackerType == "MOSSE" )
    {
        tracker = cv::legacy::TrackerMOSSE::create( );
    }
    else
    {
        std::cout << "INVALID TRACKER SPECIFIED" << '\n';
        std::cout << "Available Trackers are :" << '\n';
        for ( int i = 0; i < static_cast< int >( std::size( trackerTypes ) );
              i++ )
            std::cout << i << " : "
                      << trackerTypes[ static_cast< size_t >( i ) ] << '\n';
    }

    return tracker;
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    // Set tracker type. Change this to try different trackers.
    std::string trackerType = "MEDIANFLOW"; //"CSRT";

    std::cout << "Default tracking algoritm is CSRT" << '\n';
    std::cout << "Available tracking algorithms are:" << '\n';
    for ( std::vector< std::string >::iterator it = trackerTypes.begin( );
          it != trackerTypes.end( );
          ++it )
        std::cout << " " << *it << '\n';

    // set default values for tracking algorithm and video
    std::string videoPath = IMAGES_ROOT + "/cycle.mp4";

    // Initialize MultiTracker with tracking algo
    std::vector< cv::Rect > bboxes;

    // create a video capture object to read videos
    cv::VideoCapture cap( videoPath );
    cv::Mat frame;

    // quit if unabke to read video file
    if ( ! cap.isOpened( ) )
    {
        std::cout << "Error opening video file " << videoPath << std::endl;
    }

    // read first frame
    cap >> frame;

    showMat( frame, "Tracking", true );

    bboxes.emplace_back( 471, 250, 66, 159 );
    bboxes.emplace_back( 349, 232, 69, 102 );

    std::vector< cv::Scalar > colors;
    getRandomColors( colors, static_cast< int >( bboxes.size( ) ) );

    //// Initialize MultiTracker
    // There are two ways you can initialize multitracker
    // 1. tracker = cv2.MultiTracker("CSRT")
    // All the trackers added to this multitracker
    // will use CSRT algorithm as default
    // 2. tracker = cv2.MultiTracker()
    // No default algorithm specified

    // Initialize MultiTracker with tracking algo
    // Specify tracker type
    // Create multitracker
    cv::Ptr< cv::legacy::MultiTracker > multiTracker =
        cv::legacy::MultiTracker::create( );

    // initialize multitracker
    for ( size_t i = 0; i < bboxes.size( ); i++ )
        multiTracker->add( createTrackerByName( trackerType ),
                           frame,
                           cv::Rect2d( bboxes[ i ] ) );

    // We will display only 5 frames
    int count = 0;
    int count1 = 0;

    // process video and track objects
    while ( cap.isOpened( ) )
    {
        // get frame from the video
        cap >> frame;

        // stop the program if reached end of video
        if ( frame.empty( ) )
            break;

        // update the tracking result with new frame
        multiTracker->update( frame );

        // draw tracked objects
        for ( unsigned i = 0; i < multiTracker->getObjects( ).size( ); i++ )
        {
            rectangle(
                frame, multiTracker->getObjects( )[ i ], colors[ i ], 2, 1 );
        }

        if ( count % 10 == 0 )
        {
            showMat( frame, "Tracking", false );
            count1 += 1;
        }

        count += 1;

        if ( count == 50 )
            break;
    }

    // Clean up
    cap.release( );
    cv::destroyAllWindows( );

    return 0;
}