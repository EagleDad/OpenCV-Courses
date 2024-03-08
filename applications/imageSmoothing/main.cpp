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
    // Box Blur
    std::string filename = IMAGES_ROOT + "/gaussian-noise.png";

    cv::Mat image = cv::imread( filename );

    cv::Mat dst1, dst2;

    // Box filter- kernel size 3
    cv::blur( image, dst1, cv::Size( 3, 3 ), cv::Point( -1, -1 ) );

    // Box filter kernel size 7
    cv::blur( image, dst2, cv::Size( 7, 7 ), cv::Point( -1, -1 ) );

    showMat( image, "Original Image", false );
    showMat( dst1, "Box Blur Result 1 : KernelSize = 3", false );
    showMat( dst2, "Box Blur Result 2 : KernelSize = 7", true );

    // Gaussian Blur
    image = cv::imread( filename );

    // Apply gaussian filter
    cv::GaussianBlur( image, dst1, cv::Size( 5, 5 ), 0, 0 );

    // Increased sigma
    cv::GaussianBlur( image, dst2, cv::Size( 25, 25 ), 50, 50 );

    showMat( image, "Original Image", false );
    showMat( dst1, "Gaussian Blur Result 1 : KernelSize = 5", false );
    showMat( dst2, "Gaussian Blur Result 2 : KernelSize = 25", true );

    // Median filtering
    filename = IMAGES_ROOT + "/salt-and-pepper.png";

    image = cv::imread( filename );

    // Defining the kernel size
    int kernelSize = 5;

    cv::Mat medianBlurred;
    // Performing Median Blurring and store in numpy array "medianBlurred"
    cv::medianBlur( image, medianBlurred, kernelSize );

    cv::GaussianBlur( image, dst1, cv::Size( 5, 5 ), 0, 0 );

    showMat( image, "Original Image", false );
    showMat( dst1, "Gaussian Blur Result 1 : KernelSize = 5", false );
    showMat( medianBlurred, "Median Blur Result : KernelSize = 5", true );

    // Bilateral Filtering
    // Non-linear
    // Edge preserving
    // Noise reducing smoothing filter

    filename = IMAGES_ROOT + "/gaussian-noise.png";

    image = cv::imread( filename );

    // diameter of the pixel neighbourhood used during filtering
    int dia = 15;
    // int dia = -1;

    // Larger the value the distant colours will be mixed together
    // to produce areas of semi equal colors
    double sigmaColor = 80;

    // Larger the value more the influence of the farther placed pixels
    // as long as their colors are close enough
    // If sigma space should be used diameter needs to be -1
    double sigmaSpace = 80;

    cv::Mat bilateralFiltered;

    // Apply bilateral filter
    cv::bilateralFilter(
        image, bilateralFiltered, dia, sigmaColor, sigmaSpace );

    cv::GaussianBlur( image, dst1, cv::Size( 5, 5 ), 0, 0 );

    showMat( image, "Original Image", false );
    showMat( dst1, "Gaussian Blur Result 1 : KernelSize = 5", false );
    showMat( bilateralFiltered, "Bilateral Blur Result", true );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}