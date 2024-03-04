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
    std::string imageName = IMAGES_ROOT + "/opening.png";
    cv::Mat image = cv::imread( imageName, cv::IMREAD_GRAYSCALE );

    showMat( image, "Org. input image", false );

    // Specify Kernel Size
    int kernelSize = 10;

    // Create the kernel
    cv::Mat element = cv::getStructuringElement(
        cv::MORPH_ELLIPSE,
        cv::Size( 2 * kernelSize + 1, 2 * kernelSize + 1 ),
        cv::Point( kernelSize, kernelSize ) );

    showMat( element * 255,
             "Structuring Element : Ellipse, Kernel Size : 10",
             false,
             2 );

    cv::Mat imEroded;
    // Perform erosion
    cv::erode( image, imEroded, element, cv::Point( -1, -1 ), 1 );
    showMat( imEroded, "After Erosion Operation", false );

    cv::Mat imOpen;
    // Perform dilation
    cv::dilate( imEroded, imOpen, element, cv::Point( -1, -1 ), 1 );
    showMat( imOpen, "After Dilation Operation", false );

    // Get structuring element/kernel which will be used
    // for opening operation
    int openingSize = 3;

    // Selecting a elliptical kernel
    element = cv::getStructuringElement(
        cv::MORPH_ELLIPSE,
        cv::Size( 2 * openingSize + 1, 2 * openingSize + 1 ),
        cv::Point( openingSize, openingSize ) );

    cv::Mat imageMorphOpened;
    morphologyEx(
        image, imageMorphOpened,
        cv::MORPH_OPEN, element,
        cv::Point( -1, -1 ), 3 );

    showMat( imageMorphOpened, "After Opening Operation", true );

    imageName = IMAGES_ROOT + "/closing.png";
    // Image taken as input
    image = cv::imread( imageName, cv::IMREAD_GRAYSCALE );

    showMat( image, "Org. input image" );

    // Specify kernel size
    kernelSize = 10;
    // Create kernel
    element =
        getStructuringElement( cv::MORPH_ELLIPSE,
                               cv::Size( 2 * kernelSize + 1, 2 * kernelSize + 1 ),
                               cv::Point( kernelSize, kernelSize ) );

    cv::Mat imDilated;
    // Perform Dilation
    cv::dilate( image, imDilated, element );

    showMat( imDilated, "After Dilation Operation" );

    cv::Mat imClose;
    // Perform erosion
    cv::erode( imDilated, imClose, element );

    showMat( imClose, "After Erosion Operation", false );

    // Create a structuring element
    int closingSize = 10;
    // Selecting a elliptical kernel and storing in Mat element
    element =
        getStructuringElement( cv::MORPH_ELLIPSE,
                               cv::Size( 2 * closingSize + 1, 2 * closingSize + 1 ),
                               cv::Point( closingSize, closingSize ) );

    cv::Mat imageMorphClosed;
    cv::morphologyEx( image, imageMorphClosed, cv::MORPH_CLOSE, element );

    showMat( imageMorphClosed, "After Closing  Operation", true );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}