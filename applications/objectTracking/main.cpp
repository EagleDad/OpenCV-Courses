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

// Convert to string
#define SSTR( x )                                                              \
    static_cast< std::ostringstream >(                                        \
        ( std::ostringstream( ) << std::dec << x ) )                           \
        .str( )

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    const std::string trackerTypes[ 8 ] = { "BOOSTING",
                                            "MIL",
                                            "KCF",
                                            "TLD",
                                            "MEDIANFLOW",
                                            "GOTURN",
                                            "CSRT",
                                            "MOSSE" };

    // Create a tracker
    const std::string trackerType = trackerTypes[ 2 ];

    cv::Ptr< cv::Tracker > tracker { nullptr };
    cv::Ptr< cv::legacy::Tracker > legacyTracker { nullptr };

    if ( trackerType == "BOOSTING" )
    {
        legacyTracker = cv::legacy::TrackerBoosting::create( );
    }
    else if ( trackerType == "MIL" )
    {
        tracker = cv::TrackerMIL::create( );
    }
    else if ( trackerType == "KCF" )
    {
        tracker = cv::TrackerKCF::create( );
    }
    else if ( trackerType == "TLD" )
    {
        legacyTracker = cv::legacy::TrackerTLD::create( );
    }
    else if ( trackerType == "MEDIANFLOW" )
    {
        legacyTracker = cv::legacy::TrackerMedianFlow::create( );
    }
    else if ( trackerType == "GOTURN" )
    {
        tracker = cv::TrackerGOTURN::create( );
    }
    else if ( trackerType == "CSRT" )
    {
        tracker = cv::TrackerCSRT::create( );
    }
    else if ( trackerType == "MOSSE" )
    {
        legacyTracker = cv::legacy::TrackerMOSSE::create( );
    }
    else
    {
        std::cout << "INVALID TRACKER SPECIFIED" << '\n';
        std::cout << "Available Trackers are :" << '\n';
        for ( int i = 0; i < static_cast< int >( std::size( trackerTypes ) );
              i++ )
            std::cout << i << " : " << trackerTypes[ i ] << '\n';
        return -1;
    }

    // Read video
    cv::VideoCapture video( IMAGES_ROOT + "/hockey.mp4" );

    // Exit if video is not opened
    if ( ! video.isOpened( ) )
    {
        std::cout << "Could not read video file" << '\n';
    }

    // Read first frame
    cv::Mat frame;
    bool ok = video.read( frame );
    std::ignore = ok;

    // Define a few colors for drawing
    cv::Scalar red( 0, 0, 255 );
    cv::Scalar blue( 255, 128, 0 );

    // Define an initial bounding box
    // Cycle
    // cv::Rect2d bbox (477, 254, 55, 152);

    // ship
    // cv::Rect2d bbox (751, 146, 51, 78);

    // Hockey
    cv::Rect bbox( 129, 47, 74, 85 );

    // Face2
    // cv::Rect2d bbox (237, 145, 74, 88);

    // meeting
    // cv::Rect2d bbox (627, 183, 208, 190);     //CSRT
    // cv::Rect2d bbox (652, 187, 118, 123);       //KCF

    // surfing
    // cv::Rect2d bbox (97, 329, 118, 293);

    // surf
    // cv::Rect2d bbox (548, 587, 52, 87);

    // spinning
    // cv::Rect2d bbox (232, 218, 377, 377);       //RED
    // cv::Rect2d bbox (699, 208, 383, 391);         //BLUE

    // Car
    // cv::Rect2d bbox (71, 457, 254, 188);

    // Uncomment the line below to select a different bounding box
    // bbox = selectROI(frame, False);

    // Initialize tracker
    if ( tracker != nullptr )
    {
        tracker->init( frame, bbox );
    }

    if ( legacyTracker != nullptr )
    {
        legacyTracker->init( frame, bbox );
    }

    // Display bounding box.
    cv::rectangle( frame, bbox, blue, 2, 1 );

    showMat( frame, "Tracking", true );

    // We will display only first 5 frames
    int count = 0;

    while ( video.read( frame ) )
    {
        // Start timer
        const double timer = static_cast< double >( cv::getTickCount( ) );

        // The update method is used to obtain the location
        // of the new tracked object. The method returns
        // false when the track is lost. Tracking can fail
        // because the object went outside the video frame or
        // if the tracker failed to track the object.
        // In both cases, a false value is returned.

        // Update the tracking result
        bool success { false };

        if ( tracker != nullptr )
        {
            success = tracker->update( frame, bbox );
        }

        if ( legacyTracker != nullptr )
        {
            cv::Rect2d bBox2D( bbox.x, bbox.y, bbox.width, bbox.height );
            success = legacyTracker->update( frame, bBox2D );
        }

        // Calculate Frames per second (FPS)
        const auto fps =
            cv::getTickFrequency( ) /
            ( static_cast< float >( cv::getTickCount( ) ) - timer );

        if ( success )
        {
            // Tracking success : Draw the tracked object
            cv::rectangle( frame, bbox, cv::Scalar( 255, 0, 0 ), 2, 1 );
        }
        else
        {
            // Tracking failure detected.
            cv::putText( frame,
                         "Tracking failure detected",
                         cv::Point( 100, 80 ),
                         cv::FONT_HERSHEY_SIMPLEX,
                         0.75,
                         cv::Scalar( 0, 0, 255 ),
                         2 );
        }

        // Display tracker type on frame
        cv::putText( frame,
                     trackerType + " Tracker",
                     cv::Point( 100, 20 ),
                     cv::FONT_HERSHEY_SIMPLEX,
                     0.75,
                     cv::Scalar( 50, 170, 50 ),
                     2 );

        // Display FPS on frame
        cv::putText( frame,
                     "FPS : " + SSTR( static_cast< int >( fps ) ),
                     cv::Point( 100, 50 ),
                     cv::FONT_HERSHEY_SIMPLEX,
                     0.75,
                     cv::Scalar( 50, 170, 50 ),
                     2 );

        //showMat( frame, "Tracking " + std::to_string( count ), false );
        showMat( frame, "Tracking", false );

        count += 1;

       /* if ( count == 5 )
            break;*/
    }

    //showMat( frame, "Tracking", true );

    // Clean up
    cv::waitKey( 0 );
    cv::destroyAllWindows( );

    return 0;
}