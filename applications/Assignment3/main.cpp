#include <macros.h>

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
IGNORE_WARNINGS_POP

int maxScaleUp = 100;
int scaleFactor = 1;
int scaleType = 0;
int maxType = 1;

// Trackbar name not shown properly.
// https://github.com/opencv/opencv/issues/9489

std::string windowName = "Resize Image";
std::string trackbarValue = "Scale";
std::string trackbarType = "Type: \n 0: Scale Up \n 1: Scale Down";

// Source image
cv::Mat source;

void scaleImage( int, void* );

int main( int argc, char** argv )
{
    std::ignore = argc;
    std::ignore = argv;

    // load an image
    source = cv::imread( "truth.png" );

    // Create a window to display results
    cv::namedWindow( windowName, cv::WINDOW_AUTOSIZE );

    // Create Trackbars and associate a callback function
    cv::createTrackbar(
        trackbarValue, windowName, &scaleFactor, maxScaleUp, scaleImage );
    cv::createTrackbar(
        trackbarType, windowName, &scaleType, maxType, scaleImage );

    scaleImage( 25, nullptr );

    while ( true )
    {
        const auto cIn = cv::waitKey( 20 );

        if ( static_cast< char >( cIn ) == 27 )
            break;
    }

    return 0;
}

// Callback functions
void scaleImage( int, void* )
{
    // Get the Scale factor from the trackbar
    double scaleFactorDouble;

    if ( scaleType == 0 )
    {
        scaleFactorDouble = 1 + scaleFactor / 100.0;
    }
    else
    {
        scaleFactorDouble = 1 - scaleFactor / 100.0;
    }

    if ( scaleFactorDouble == 0 )
    {
        scaleFactorDouble = 1.0;
    }

    cv::Mat scaledImage;

    // Resize the image
    cv::resize( source,
                scaledImage,
                cv::Size( ),
                scaleFactorDouble,
                scaleFactorDouble,
                cv::INTER_LINEAR );

    cv::imshow( windowName, scaledImage );
}