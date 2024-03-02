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
    cv::VideoCapture cap( IMAGES_ROOT + "/chaplin.mp4" );

    // Check if camera opened successfully
    if ( ! cap.isOpened( ) )
    {
        std::cout << "Error opening video stream or file" << '\n';
    }

    // Read until video is completed
    while ( cap.isOpened( ) )
    {
        cv::Mat frame;

        // Capture frame-by-frame
        cap >> frame;

        // If the frame is empty, break immediately
        if ( frame.empty( ) )
            break;

        // Write the frame into the file 'outputChaplin.mp4'
        // Wait for 25 ms before moving on to the next frame
        // This will slow down the video
        showMat( frame, "Frame", false, 1, 25 );
    }

    cap.release( );

    // Let's create the VideoCapture object
    cv::VideoCapture cap2( IMAGES_ROOT + "/chaplin.mp4" );

    auto width = cap2.get( cv::CAP_PROP_FRAME_WIDTH );
    auto height = cap2.get( cv::CAP_PROP_FRAME_HEIGHT );

    std::cout << width << "," << height << "\n";

    // Read the first frame
    cv::Mat frame;
    cap2 >> frame;

    showMat( frame, "First video frame", false );

    // Set position of video to 2.5 seconds
    cap2.set( cv::CAP_PROP_POS_MSEC, 2500 );

    // Width
    cap2.set( cv::CAP_PROP_FRAME_WIDTH, 320 );

    // Height
    cap2.set( cv::CAP_PROP_FRAME_HEIGHT, 180 );

    cap2 >> frame;

    showMat( frame, "Second video frame", false );

    std::cout << frame.size( ) << "\n";

    cap2.release( );

    // Write a Video
    cv::VideoCapture cap3( IMAGES_ROOT + "/chaplin.mp4" );

    // Check if camera opened successfully
    if ( ! cap3.isOpened( ) )
    {
        std::cout << "Error opening video stream or file" << '\n';
    }

    // Default resolutions of the frame are obtained.The default resolutions are
    // system dependent.
    int32_t frame_width =
        static_cast< int32_t >( cap3.get( cv::CAP_PROP_FRAME_WIDTH ) );
    int32_t frame_height =
        static_cast< int32_t >( cap3.get( cv::CAP_PROP_FRAME_HEIGHT ) );

    // Define the codec and create VideoWriter object.
    // The output is stored in 'outputChaplin.mp4' file.
    cv::VideoWriter out( RESULTS_ROOT + "/outputChaplin.mp4",
                         cv::VideoWriter::fourcc( 'M', 'J', 'P', 'G' ),
                         10,
                         cv::Size( frame_width, frame_height ) );

    // Read until video is completed
    while ( cap3.isOpened( ) )
    {
        cv::Mat frameRead;

        // Capture frame-by-frame
        cap3 >> frameRead;

        // If the frame is empty, break immediately
        if ( frameRead.empty( ) )
            break;

        // Write the frame into the file 'outputChaplin.mp4'
        out.write( frameRead );

        // Write the frame into the file 'outputChaplin.mp4'
        // Wait for 25 ms before moving on to the next frame
        // This will slow down the video
        showMat( frameRead, "FrameOut", false, 1, 25 );
    }

    cv::waitKey( 0 );

    // When everything done, release the VideoCapture and VideoWriter objects
    cap3.release( );
    out.release( );

    cv::destroyAllWindows( );

    return 0;
}