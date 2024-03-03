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
    std::string imageName = IMAGES_ROOT + "/dilation_example.jpg";

    // Read the input image
    cv::Mat image = cv::imread( imageName, cv::IMREAD_COLOR );

    showMat( image, "Input image", false );

    int kSize = 7;
    cv::Mat kernel1 = cv::getStructuringElement( cv::MORPH_ELLIPSE,
                                                 cv::Size( kSize, kSize ) );

    showMat( kernel1 * 255, "Structure Element 1", false );

    cv::Mat imageDilated;
    cv::dilate( image, imageDilated, kernel1 );

    showMat( imageDilated, "Dilated Image", false );

    kSize = 3;
    cv::Mat kernel2 =
        getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( kSize, kSize ) );

    showMat( kernel2 * 255, "Structure Element 1", false, 4 );

    cv::Mat imageDilated1, imageDilated2;
    cv::dilate( image, imageDilated1, kernel2, cv::Point( -1, -1 ), 1 );
    cv::dilate( image, imageDilated2, kernel2, cv::Point( -1, -1 ), 2 );

    showMat( imageDilated1, "Dilated Image 1", false );
    showMat( imageDilated2, "Dilated Image 2", true );

    // Erosion
    imageName = IMAGES_ROOT + "/erosion_example.jpg";

    // Read the input image
    image = cv::imread( imageName, cv::IMREAD_COLOR );

    showMat( image, "Input image", false );

    cv::Mat imageEroded;

    // First parameter is the original image, second is the eroded image
    cv::erode( image, imageEroded, kernel1 );

    showMat( imageEroded, "Eroded Image", true );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}