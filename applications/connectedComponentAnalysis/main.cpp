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
    // Read image as grayscale
    cv::Mat img =
        cv::imread( IMAGES_ROOT + "/truth.png", cv::IMREAD_GRAYSCALE );

    showMat( img, "Input Image", false );

    // Threshold Image
    cv::Mat imThresh;
    cv::threshold( img, imThresh, 127, 255, cv::THRESH_BINARY );

    // Find connected components
    cv::Mat imLabels;
    int nComponents = cv::connectedComponents( imThresh, imLabels );

    cv::Mat imLabelsCopy = imLabels.clone( );

    // First let's find the min and max values in imLabels
    cv::Point minLoc, maxLoc;
    double minVal, maxVal;

    // The following line finds the min and max pixel values
    // and their locations in an image.
    cv::minMaxLoc( imLabels, &minVal, &maxVal, &minLoc, &maxLoc );

    // Normalize the image so the min value is 0 and max value is 255.
    imLabels = 255 * ( imLabels - minVal ) / ( maxVal - minVal );

    // Convert image to 8-bits
    imLabels.convertTo( imLabels, CV_8U );

    showMat( imLabels, "Label Image", false );

    imLabels = imLabelsCopy.clone( );
    // Display the labels
    std::cout << "Number of components = " << nComponents << "\n";

    showMat( imLabels == 0, "Label Image 0", false );
    showMat( imLabels == 1, "Label Image 1", false );
    showMat( imLabels == 2, "Label Image 2", false );
    showMat( imLabels == 3, "Label Image 3", false );
    showMat( imLabels == 4, "Label Image 4", false );
    showMat( imLabels == 5, "Label Image 5", false );

    // Make a copy of the image
    imLabels = imLabelsCopy.clone( );

    // First let's find the min and max values in imLabels
    // The following line finds the min and max pixel values
    // and their locations in an image.
    double minValue2, maxValue2;
    cv::minMaxLoc( imLabels, &minValue2, &maxValue2, &minLoc, &maxLoc );

    // Normalize the image so the min value is 0 and max value is 255.
    imLabels = 255 * ( imLabels - minValue2 ) / ( maxValue2 - minValue2 );

    // Convert image to 8-bits
    imLabels.convertTo( imLabels, CV_8U );

    // Apply a color map
    cv::Mat imColorMap;
    cv::applyColorMap( imLabels, imColorMap, cv::COLORMAP_JET );

    // Display colormapped labels
    showMat( imColorMap, "Label Image Color", true );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}