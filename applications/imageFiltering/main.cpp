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
    std::string filename = IMAGES_ROOT + "/sample.jpg";

    cv::Mat image = cv::imread( filename );

    if ( image.empty( ) )
    {
        std::cout << "Could not read image" << '\n';
    }

    /*
     * First, we need to define a kernel. For this exercise, let’s use a kernel
     * size of 5x5. To keep things simple, we will also choose a kernel where
     * all elements are the same. This kernel is often called a Box Kernel.
     * Notice, the sum of all the elements of the kernel add up to 1. This
     * ensures that the output image has the same brightness level as the input
     * image. If we do not do this, the output pixels will be approximately 25
     * times (5x5 = 25) brighter than the input pixels on average.
     */

    // Set kernel size to 5
    int kernelSize = 5;

    // Create a 5x5 kernel with all elements equal to 1
    cv::Mat kernel = cv::Mat::ones( kernelSize, kernelSize, CV_32F );

    // Normalize kernel so sum of all elements equals 1
    kernel = kernel / static_cast< float >( kernelSize * kernelSize );

    // Print kernel
    std::cout << kernel << '\n';

    // Output  image
    cv::Mat result;

    // Apply filter
    cv::filter2D(
        image, result, -1, kernel, cv::Point( -1, -1 ), 0, cv::BORDER_DEFAULT );

    showMat( image, "Original Image", false );
    showMat( result, "Convolution Result", true );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}