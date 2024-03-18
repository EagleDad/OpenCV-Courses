#include <GUI.h>
#include <macros.h>

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
// #include <opencv2/stitching.hpp>
IGNORE_WARNINGS_POP

// STD includes
#include <iostream>

const std::string IMAGES_ROOT = "C:/images";
const std::string RESULTS_ROOT = "C:/images/results";

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    std::string dirName = IMAGES_ROOT + "/scene/";
    std::vector< cv::Mat > images;
    std::vector< cv::String > files;
    cv::String destination = "panorama.jpg";

    cv::glob( dirName, files );
    std::sort( files.begin( ), files.end( ) );

    for ( size_t i = 0; i < files.size( ); ++i )
    {
        cv::Mat img = cv::imread( files[ i ] ); // load the image
        if ( img.empty( ) )                     // invalid image, skip it.
        {
            std::cout << files[ i ] << " is invalid!" << std::endl;
            continue;
        }

        // Create a list of images
        images.push_back( img );

        showMat( img, "Image " + std::to_string( i ), true );
    }

    ///
    /// YOUR CODE HERE
    ///

    // Define a image for the final output
    cv::Mat imgPanorama;

    // Create a stitcher for panorama images
    const cv::Ptr< cv::Stitcher > stitcher =
        cv::Stitcher::create( cv::Stitcher::PANORAMA );

    // Call the stitcher to stitch all the images in the image array
    const cv::Stitcher::Status status = stitcher->stitch( images, imgPanorama );

    if ( status != cv::Stitcher::OK )
    {
        std::cout << "Can't stitch images\n";
        return -1;
    }

    // Store a new image stitched from the given
    // set of images as "result.jpg"
    cv::imwrite( RESULTS_ROOT + "\result.jpg", imgPanorama );

    showMat( imgPanorama, "Panorama", true );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}