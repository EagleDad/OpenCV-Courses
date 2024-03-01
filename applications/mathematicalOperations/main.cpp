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
    // Data type conversion
    // Read image
    cv::Mat image = cv::imread( IMAGES_ROOT + "/boy.png" );
    // Check image type.
    std::cout << "Image Data Type : " << image.type( ) << '\n';

    showMat( image, "boy", true );

    // Convert to CV_16U
    cv::Mat image_16U = image.clone( );
    image_16U.convertTo( image_16U, CV_16U );
    showMat( image_16U, "boy 16U", true );

    // Convert to CV_32F
    cv::Mat image_32F = image.clone( );
    image_32F.convertTo( image_32F, CV_32F );
    showMat( image_32F, "boy 32F", true );

    // Map the pixel intensities to [0,1] range.
    image_32F = image_32F / 255.0;

    showMat( image_32F, "boy 32F mapped", true );
    

    return 0;
}