#include <macros.h>

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
IGNORE_WARNINGS_POP

// STD includes
#include <iostream>

const std::string IMAGES_ROOT = "C:/images";
const std::string RESULTS_ROOT = "C:/images/results";

//
// Function declarations
//
cv::Rect detectSoccerBall( const cv::Mat& mat );
cv::Ptr< cv::Tracker > getTracker( const std::string& trackerType );

//
// Global variables
//
int32_t minObjectArea = 1500;
double minCircularity = 0.85;
int32_t maxBboxWidth = 250;
int32_t minBboxWidth = 90;
const std::string trackerTypes[ 8 ] = { "MIL", "KCF", "CSRT" };

//
// UI Interface
//
std::string windowName = "Tracking";

//
//
//

// Convert to string
#define SSTR( x )                                                              \
    static_cast< std::ostringstream >(                                         \
        ( std::ostringstream( ) << std::dec << x ) )                           \
        .str( )

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    // Create a tracker
    const std::string trackerType = trackerTypes[ 1 ];

    cv::Ptr< cv::Tracker > tracker = getTracker( trackerType );

    const std::string filename = IMAGES_ROOT + "/soccer-ball.mp4";
    cv::VideoCapture videoCap( filename );

    if ( ! videoCap.isOpened( ) )
    {
        std::cout << "Error opening video stream or file\n";
        return -1;
    }

    cv::Mat frame;
    videoCap >> frame;

    // Detect the initial soccer ball for the following tracking
    const auto initialRect = detectSoccerBall( frame );

    // Initialize tracker
    if ( tracker != nullptr )
    {
        tracker->init( frame, initialRect );
    }

    cv::rectangle( frame, initialRect, cv::Scalar( 0, 255, 0 ), 3 );

    cv::Rect currWindow = initialRect;

    cv::namedWindow( windowName );
    cv::imshow( windowName, frame );

    while ( true )
    {
        // Read frame
        videoCap >> frame;

        if ( frame.empty( ) )
        {
            break;
        }

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
            success = tracker->update( frame, currWindow );
        }

        // Calculate Frames per second (FPS)
        const auto fps =
            cv::getTickFrequency( ) /
            ( static_cast< float >( cv::getTickCount( ) ) - timer );

        if ( success )
        {
            // Tracking success : Draw the tracked object
            cv::rectangle( frame, currWindow, cv::Scalar( 255, 0, 0 ), 2, 1 );
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

            // Reset the tracker
            const auto newRect = detectSoccerBall( frame );

            if ( newRect.size( ).width > 0 )
            {
                tracker = getTracker( trackerType );

                // Initialize tracker
                if ( tracker != nullptr )
                {
                    tracker->init( frame, newRect );
                }
            }
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

        cv::imshow( windowName, frame );
        cv::waitKey( 10 );
    }

    // Clean up
    videoCap.release( );
    cv::destroyAllWindows( );

    return 0;
}

cv::Rect detectSoccerBall( const cv::Mat& mat )
{
    // Convert image to gray scale
    cv::Mat imageGray;

    cv::cvtColor( mat, imageGray, cv::COLOR_BGR2GRAY );

    // Apply median filter to remove some noise from the image
    cv::medianBlur( imageGray, imageGray, 5 );

    // The soccer ball should appear bright in th image, so we use OTS to detect
    // it.
    cv::Mat imgSegmented;
    cv::threshold(
        imageGray, imgSegmented, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU );

    // Find all contours in the image
    std::vector< std::vector< cv::Point > > contours;
    std::vector< cv::Vec4i > hierarchy;

    cv::findContours( imgSegmented,
                      contours,
                      hierarchy,
                      cv::RETR_EXTERNAL,
                      cv::CHAIN_APPROX_SIMPLE );

    // Remove small contours
    auto cntBegin = contours.begin( );

    for ( int i = 0; cntBegin != contours.end( ); i++ )
    {
        const auto area = cv::contourArea( *cntBegin );

        if ( area < minObjectArea )
        {
            cntBegin = contours.erase( cntBegin );
        }
        else
        {
            ++cntBegin;
        }
    }

    // The soccer ball is a round object, so we search for the contour with
    // maximum circularity

    /*
     * *           A
     * C' = ----------- := C = min(1, C')
     *      (max^2 * PI)
     *
     *  With A = object area
     *  and max ist the biggest radius of the object
     */

    size_t bestIdx { };
    double maxCircularity = std::numeric_limits< double >::min( );

    for ( size_t i = 0; i < contours.size( ); i++ )
    {
        const auto area = cv::contourArea( contours[ i ] );
        const auto minAreaRect = cv::minAreaRect( contours[ i ] );

        const auto max = std::max( minAreaRect.size.width / 2.0,
                                   minAreaRect.size.height / 2.0 );

        const auto circularity = area / ( max * max * CV_PI );

        if ( circularity > maxCircularity )
        {
            maxCircularity = circularity;
            bestIdx = i;
        }
    }

    const auto bbox = cv::boundingRect( contours[ bestIdx ] );

    if ( maxCircularity < minCircularity || bbox.width > maxBboxWidth ||
         bbox.width < minBboxWidth )
    {
        return { };
    }

    std::cout << "Circularity: " << std::to_string( maxCircularity ) << '\n';
    std::cout << "Width: " << std::to_string( bbox.width ) << '\n';

    return bbox;
}

cv::Ptr< cv::Tracker > getTracker( const std::string& trackerType )
{
    if ( trackerType == "MIL" )
    {
        return cv::TrackerMIL::create( );
    }

    if ( trackerType == "KCF" )
    {
        return cv::TrackerKCF::create( );
    }

    if ( trackerType == "CSRT" )
    {
        return cv::TrackerCSRT::create( );
    }

    std::cout << "INVALID TRACKER SPECIFIED" << '\n';
    std::cout << "Available Trackers are :" << '\n';

    for ( int i = 0; i < static_cast< int >( std::size( trackerTypes ) ); i++ )
    {
        std::cout << i << " : " << trackerTypes[ i ] << '\n';
    }

    return nullptr;
}
