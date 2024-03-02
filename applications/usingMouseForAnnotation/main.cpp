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
    // Open webcam
    cv::VideoCapture cap( 0 );
    cv::Mat frame;
    int k = 0;
    // Detect if webcam is working properly
    if ( ! cap.isOpened( ) )
    {
        std::cout << "Unable to detect webcam "
                  << "\n";
        return 0;
    }
    else
    {
        while ( 1 )
        {
            // Capture frame
            cap >> frame;
            if ( k == 27 )
                break;
            // The following if-else block is used to check which key is
            // pressed. We use the waitKey() function to detect the input and
            // respond only if either ‘e’ or ‘z’ is pressed.
            // ‘ESC’(ASCII code = 27) is used to exit the program.

            // Identify if 'e' or 'E' is pressed
            if ( k == 101 || k == 69 )
                putText( frame,
                         "E is pressed, press Z or ESC",
                         cv::Point( 100, 180 ),
                         cv::FONT_HERSHEY_SIMPLEX,
                         1,
                         cv::Scalar( 0, 255, 0 ),
                         3 );
            // Identify if 'z' or 'Z' is pressed or not
            if ( k == 90 || k == 122 )
                putText( frame,
                         "Z is pressed",
                         cv::Point( 100, 180 ),
                         cv::FONT_HERSHEY_SIMPLEX,
                         1,
                         cv::Scalar( 0, 255, 0 ),
                         3 );
            imshow( "Image", frame );
            // Waitkey is increased so that the display is shown
            k = cv::waitKey( 10000 ) & 0xFF;
        }
    }

    cap.release( );
    cv::waitKey( 0 );
    cv::destroyAllWindows( );

    return 0;
}