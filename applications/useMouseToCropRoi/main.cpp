#include <macros.h>

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
IGNORE_WARNINGS_POP

const std::string IMAGES_ROOT = "C:/images";
const std::string RESULTS_ROOT = "C:/images/results";

// Points to store the top left and bottom right position of the ROI
cv::Point topLeft, bottomRight;
// Source image
cv::Mat source;
// Run variable that will stop the app after storing the ROI
bool run = true;

// function which will be called on mouse input
void drawCircle( int action, int x, int y, [[maybe_unused]] int flags,
                 [[maybe_unused]] void* userdata )
{
    // Action to be taken when left mouse button is pressed
    if ( action == cv::EVENT_LBUTTONDOWN )
    {
        topLeft = cv::Point( x, y );

        // Mark the top left position
        cv::circle(
            source, topLeft, 1, cv::Scalar( 255, 0, 0 ), 5, cv::LINE_AA );
    }
    // Action to be taken when left mouse button is released
    else if ( action == cv::EVENT_LBUTTONUP )
    {
        bottomRight = cv::Point( x, y );

        // Mark the top bottom position
        cv::circle(
            source, bottomRight, 1, cv::Scalar( 255, 0, 0 ), 5, cv::LINE_AA );

        // Draw the bounding rect
        cv::rectangle( source,
                       topLeft,
                       bottomRight,
                       cv::Scalar( 0, 255, 0 ),
                       2,
                       cv::LINE_AA );

        imshow( "Window", source );

        const cv::Mat roi = source( cv::Rect( topLeft, bottomRight ) );

        cv::imshow( "ROI", roi );

        cv::imwrite( RESULTS_ROOT + "/face.png", roi );

        run = false;
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
    while ( k != 27 && run )
    {
        imshow( "Window", source );

        putText( source,
                 "Choose top left and drag.",
                 cv::Point( 10, 30 ),
                 cv::FONT_HERSHEY_SIMPLEX,
                 0.7,
                 cv::Scalar( 255, 255, 255 ),
                 2 );

        k = cv::waitKey( 20 ) & 0xFF;
    }

    cv::waitKey( 0 );
    cv::destroyAllWindows( );

    return 0;
}