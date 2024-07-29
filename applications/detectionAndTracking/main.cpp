#include "GUI.h"

#include <macros.h>

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
IGNORE_WARNINGS_POP

// STD includes
#include <fstream>
#include <iostream>

const std::string IMAGES_ROOT = "C:/images";
const std::string MODELS_ROOT = "C:/images/models";
const std::string RESULTS_ROOT = "C:/images/results";

enum class Detector
{
    Simple,
    DeepLearning
};

//
// Function declarations
//
cv::Rect detectSoccerBallSimple( const cv::Mat& mat );
cv::Rect detectSoccerBallDl( const cv::dnn::Net& net, cv::Mat& mat );
cv::Ptr< cv::Tracker > getTracker( const std::string& trackerType );
cv::Mat detect_objects( cv::dnn::Net net, const cv::Mat& frame );
cv::Rect detectSoccerBall( const cv::dnn::Net& net, cv::Mat& mat,
                           Detector detector );

//
// Global variables
//
int32_t minObjectArea = 1500;
double minCircularity = 0.85;
int32_t maxBboxWidth = 250;
int32_t minBboxWidth = 90;
const std::string trackerTypes[ 8 ] = { "MIL", "KCF", "CSRT" };
constexpr size_t inWidth = 300;
constexpr size_t inHeight = 300;
constexpr double inScaleFactor = 1.0 / 127.5;
const cv::Scalar meanVal( 127.5, 127.5, 127.5 );
std::vector< std::string > classes;
double decisionThreshold = 0.25;

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
    constexpr Detector detetor = Detector::Simple;

    // Create a tracker
    const std::string trackerType = trackerTypes[ 1 ];

    cv::Ptr< cv::Tracker > tracker = getTracker( trackerType );

    //
    // Single Shot Multibox Detector
    //
    const std::string configFile =
        MODELS_ROOT + "/ssd_mobilenet_v2_coco_2018_03_29.pbtxt";

    const std::string modelFile =
        MODELS_ROOT +
        "/ssd_mobilenet_v2_coco_2018_03_29/frozen_inference_graph.pb";

    const std::string classFile = MODELS_ROOT + "/coco_class_labels.txt";

    //
    // Read Tensorflow Model
    //

    cv::dnn::Net net = cv::dnn::readNetFromTensorflow( modelFile, configFile );

    //
    // Check Class Labels
    //

    std::ifstream ifs( classFile.c_str( ) );
    std::string line;
    while ( std::getline( ifs, line ) )
    {
        classes.push_back( line );
    }

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
    const auto initialRect = detectSoccerBall( net, frame, detetor );

    // Initialize tracker
    if ( tracker != nullptr )
    {
        tracker->init( frame, initialRect );
    }

    cv::rectangle( frame, initialRect, cv::Scalar( 255, 0, 0 ), 3 );

    cv::Rect currWindow = initialRect;

    cv::namedWindow( windowName );
    cv::imshow( windowName, frame );
    cv::waitKey( 250 );

    while ( true )
    {
        int32_t waitTime = 10;

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
            cv::rectangle( frame, currWindow, cv::Scalar( 0, 255, 0 ), 2, 1 );
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
            const auto newRect = detectSoccerBall( net, frame, detetor );

            if ( newRect.size( ).width > 0 )
            {
                tracker = getTracker( trackerType );

                // Initialize tracker
                if ( tracker != nullptr )
                {
                    tracker->init( frame, newRect );
                }

                cv::rectangle( frame, newRect, cv::Scalar( 255, 0, 0 ), 2, 1 );
                waitTime = 250;
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
        cv::waitKey( waitTime );
    }

    // Clean up
    videoCap.release( );
    cv::destroyAllWindows( );

    return 0;
}

cv::Rect detectSoccerBallSimple( const cv::Mat& mat )
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

cv::Rect detectSoccerBallDl( const cv::dnn::Net& net, cv::Mat& mat )
{
    //
    // Results
    //
    const cv::Mat objects = detect_objects( net, mat );

    cv::Rect boundingRect;
    double bestScore = std::numeric_limits< double >::min( );

    // For every detected object
    for ( int i = 0; i < objects.rows; i++ )
    {
        const int classId = static_cast< int >( objects.at< float >( i, 1 ) );
        std::ignore = classId;
        const float score = objects.at< float >( i, 2 );

        // Recover original coordinates from normalized coordinates
        const int x = static_cast< int >( objects.at< float >( i, 3 ) *
                                          static_cast< float >( mat.cols ) );
        const int y = static_cast< int >( objects.at< float >( i, 4 ) *
                                          static_cast< float >( mat.rows ) );
        const int w = static_cast< int >( objects.at< float >( i, 5 ) *
                                              static_cast< float >( mat.cols ) -
                                          static_cast< float >( x ) );
        const int h = static_cast< int >( objects.at< float >( i, 6 ) *
                                              static_cast< float >( mat.rows ) -
                                          static_cast< float >( y ) );

        // Check if the detection is of good quality
        if ( score > decisionThreshold && classId == 37 )
        {
            if ( score > bestScore )
            {
                bestScore = score;
                boundingRect = cv::Rect( x, y, w, h );
            }
        }
    }

    return boundingRect;
}

cv::Mat detect_objects( cv::dnn::Net net, const cv::Mat& frame )
{
    const cv::Mat inputBlob =
        cv::dnn::blobFromImage( frame,
                                inScaleFactor,
                                cv::Size( inWidth, inHeight ),
                                meanVal,
                                true,
                                false );
    net.setInput( inputBlob );
    cv::Mat detection = net.forward( "detection_out" );
    cv::Mat detectionMat( detection.size[ 2 ],
                          detection.size[ 3 ],
                          CV_32F,
                          detection.ptr< float >( ) );

    return detectionMat;
}

cv::Rect detectSoccerBall( const cv::dnn::Net& net, cv::Mat& mat,
                           Detector detector )
{
    switch ( detector )
    {
    case Detector::DeepLearning:
        return detectSoccerBallDl( net, mat );

    case Detector::Simple:
        return detectSoccerBallSimple( mat );
    }

    return { };
}
