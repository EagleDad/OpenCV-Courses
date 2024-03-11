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

// Declare Mat objects for original image and mask for inpainting
cv::Mat img, inpaintMask;
// Mat object for result output
cv::Mat res;
cv::Point prevPt( -1, -1 );

// onMouse function for Mouse Handling
// Used to draw regions required to inpaint
static void onMouse( int event, int x, int y, int flags, void* )
{
    if ( event == cv::EVENT_LBUTTONUP || ! ( flags & cv::EVENT_FLAG_LBUTTON ) )
    {
        prevPt = cv::Point( -1, -1 );
    }
    else if ( event == cv::EVENT_LBUTTONDOWN )
    {
        prevPt = cv::Point( x, y );
    }
    else if ( event == cv::EVENT_MOUSEMOVE &&
              ( flags & cv::EVENT_FLAG_LBUTTON ) )
    {
        cv::Point pt( x, y );

        if ( prevPt.x < 0 )
        {
            prevPt = pt;
        }

        cv::line( inpaintMask, prevPt, pt, cv::Scalar::all( 255 ), 5, 8, 0 );
        cv::line( img, prevPt, pt, cv::Scalar( 0, 255, 0 ), 5, 8, 0 );

        prevPt = pt;

        cv::imshow( "image", img );
        cv::imshow( "image: mask", inpaintMask );
    }
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    std::string filename = IMAGES_ROOT + "/Lincoln.jpg";
    // Read image in color mode
    img = cv::imread( filename, cv::IMREAD_COLOR );
    cv::Mat img_mask;
    // Return error if image not read properly
    if ( img.empty( ) )
    {
        std::cout << "Failed to load image: " << filename << '\n';
        return 0;
    }

    cv::namedWindow( "image", cv::WINDOW_AUTOSIZE );

    // Create a copy for the original image
    img_mask = img.clone( );
    // Initialize mask (black image)
    inpaintMask = cv::Mat::zeros( img_mask.size( ), CV_8U );

    // Show the original image
    cv::imshow( "image", img );
    cv::imshow( "image: mask", inpaintMask );
    cv::setMouseCallback( "image", onMouse, nullptr );

    for ( ;; )
    {
        char c = static_cast< char >( cv::waitKey( ) );

        if ( c == 't' )
        {
            // Use Algorithm proposed by Alexendra Telea
            cv::inpaint( img, inpaintMask, res, 3, cv::INPAINT_TELEA );
            cv::imshow( "Inpaint Output using FMM", res );
        }

        if ( c == 'n' )
        {
            // Use Algorithm proposed by Bertalmio et. al.
            cv::inpaint( img, inpaintMask, res, 3, cv::INPAINT_NS );
            cv::imshow( "Inpaint Output using NS Technique", res );
        }

        if ( c == 'r' )
        {
            inpaintMask = cv::Scalar::all( 0 );
            img_mask.copyTo( img );
            cv::imshow( "image", inpaintMask );
        }
        if ( c == 27 )
        {
            break;
        }
    }

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}