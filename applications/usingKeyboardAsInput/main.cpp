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

// Points to store the center of the circle and a point on the circumference
cv::Point center, circumference;
// Source image
cv::Mat source;

// function which will be called on mouse input
void drawCircle( int action, int x, int y, [[maybe_unused]] int flags,
                 [[maybe_unused]] void* userdata )
{
    // Action to be taken when left mouse button is pressed
    if ( action == cv::EVENT_LBUTTONDOWN )
    {
        center = cv::Point( x, y );
        // Mark the center
        cv::circle(
            source, center, 1, cv::Scalar( 255, 255, 0 ), 2, cv::LINE_AA );
    }
    // Action to be taken when left mouse button is released
    else if ( action == cv::EVENT_LBUTTONUP )
    {
        circumference = cv::Point( x, y );
        // Calculate radius of the circle
        float radius = static_cast< float >(
            sqrt( pow( center.x - circumference.x, 2 ) +
                  pow( center.y - circumference.y, 2 ) ) );
        // Draw the circle
        cv::circle( source,
                    center,
                    static_cast< int32_t >( radius ),
                    cv::Scalar( 0, 255, 0 ),
                    2,
                    cv::LINE_AA );
        // imshow( "Window", source );
        showMat( source, "Window" );
    }
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    source = cv::imread( IMAGES_ROOT + "/sample.jpg", 1 );
    // Make a dummy image, will be useful to clear the drawing
    cv::Mat dummy = source.clone( );
    cv::namedWindow( "Window" );
    // highgui function called when mouse events occur
    cv::setMouseCallback( "Window", drawCircle );
    int k = 0;
    // loop until escape character is pressed
    while ( k != 27 )
    {
        imshow( "Window", source );
        putText( source,
                 "Choose center, and drag, Press ESC to exit and c to clear",
                 cv::Point( 10, 30 ),
                 cv::FONT_HERSHEY_SIMPLEX,
                 0.7,
                 cv::Scalar( 255, 255, 255 ),
                 2 );

        k = cv::waitKey( 20 ) & 0xFF;

        if ( k == 99 )
        {
            // Another way of cloning
            dummy.copyTo( source );
        }
    }

    cv::waitKey( 0 );
    cv::destroyAllWindows( );

    return 0;
}