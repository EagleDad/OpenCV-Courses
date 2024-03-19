#include "Interface.h"

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

// Instructions
static void help( )
{
    std::cout
        << "==================================================================="
           "====="
           "\n Interactive Image Segmentation using GrabCut algorithm.\n "
           "This sample shows interactive image segmentation using grabcut "
           "algorithm.\n"
           "\nUSAGE :\n"
           "\t./grabcut <filename>\n"
           "README FIRST :\n"
           "\tSelect a rectangular area around the object you want to "
           "segment.\nThen press -n to segment the object (once or a few times)"
           "\nFor any finer touch-ups,  you can press any of the keys below "
           "and draw lines on"
           "\nthe areas you want. Then again press -n for updating the "
           "output.\n"
        << "\nHot keys: \n"
           "\tESC - To Exit\n"
           "\tr - To reset the setup\n"
           "\tn - To update the segmentation\n"
           "\n"
           "\tKEY 0 - To select areas of sure background 1\n"
           "\tKEY 1 - To select areas of sure foreground 2\n"
           "\n"
           "\tKEY 2 - To select areas of probable background 3\n"
           "\tKEY 3 - To select areas of probable foreground 4\n"
           "==================================================================="
           "===="
        << '\n';
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    // Reading file from command line

    cv::CommandLineParser parser( argc, argv, "{help h||}{@input||}" );
    if ( parser.has( "help" ) )
    {
        help( );
        return 0;
    }

    std::string filename = parser.get< std::string >( "@input" );

    if ( filename.empty( ) )
    {
        help( );
        std::cout << "\n >Empty filename \n"
                     "Call Program by :"
                     "\t./grabcut <image_name>"
                  << std::endl;
        return 0;
    }

    cv::Mat image = cv::imread( filename, 1 );
    if ( image.empty( ) )
    {
        std::cout << "\n >Couldn't read image filename " << filename
                  << std::endl;
        return 1;
    }

    help( );

    const std::string winName = "image";
    cv::namedWindow( winName, cv::WINDOW_AUTOSIZE );
    cv::setMouseCallback( winName, on_mouse, 0 );
    app.setImageAndWinName( image, winName );
    app.showImage( );

    // Key bindings
    for ( ;; )
    {
        char key = ( char )cv::waitKey( 0 );
        switch ( key )
        {
        case '0': // BGD
            global_value = 1;
            std::cout
                << " Using BLUE color,  >mark background regions with left "
                   "mouse button \n"
                << std::endl;
            break;
        case '1': // FGD
            global_value = 2;
            std::cout << " Using RED color, >mark foreground regions with left "
                         "mouse button \n"
                      << std::endl;
            break;
        case '2': // prBGD
            global_value = 3;
            std::cout
                << " Using LIGHTBLUE color, >mark probable Background regions "
                   "with left mouse button \n"
                << std::endl;
            break;
        case '3': // prFGD
            global_value = 4;
            std::cout
                << " Using PINK color, >mark probable foreground regions with "
                   "left mouse button \n"
                << std::endl;
            break;
        case '\x1b': // Exit
            std::cout << "Exiting ..." << std::endl;
            goto exit_main;
        case 'r': // Reset
            std::cout << "resetting \n" << std::endl;
            app.reset( );
            app.showImage( );
            break;

        case 'n':
            int iterCount = app.getIterCount( );
            std::cout
                << iterCount + 1
                << "> For finer touchups,  mark foreground and background "
                   "after pressing keys 0-3 and again press n \n"
                << std::endl;
            int newIterCount = app.nextIter( );
            if ( newIterCount > iterCount ) // Counting Iteration for changes
            {
                app.showImage( );
            }
            else
                std::cout << "rect must be determined>" << std::endl;
            break;
        }
    }

exit_main:
    cv::destroyWindow( winName );
    return 0;

    return 0;
}