#include "macros.h"

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
IGNORE_WARNINGS_POP

// STD includes
#include <iostream>

std::string type2str( int type )
{
    std::string r;

    const auto depth = type & CV_MAT_DEPTH_MASK;
    const auto chans = static_cast<char>(1 + ( type >> CV_CN_SHIFT ));

    switch ( depth )
    {
    case CV_8U:
        r = "8U";
        break;
    case CV_8S:
        r = "8S";
        break;
    case CV_16U:
        r = "16U";
        break;
    case CV_16S:
        r = "16S";
        break;
    case CV_32S:
        r = "32S";
        break;
    case CV_32F:
        r = "32F";
        break;
    case CV_64F:
        r = "64F";
        break;
    default:
        r = "User";
        break;
    }

    r += "C";
    r += ( chans + '0' );

    return r;
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    const std::string imagePath = "../../../images/number_zero.jpg";

    // Read image in Grayscale format
    const cv::Mat testImage = cv::imread( imagePath, 0 );

    std::cout << testImage << std::endl;

    std::cout << "Data type = " << type2str( testImage.type( ) ) << std::endl;
    std::cout << "Image Dimensions = " << testImage.size( ) << std::endl;

    return 0;
}