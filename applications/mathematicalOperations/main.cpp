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

    // Contrast enhancement
    // I0 = aI
    double contrastPercentage = 50.0;

    cv::Mat image32F = image.clone( );
    cv::Mat highCont_1;
    cv::Mat highCont_2;

    // Convert to CV_64F without mapping to [0,1].
    image32F.convertTo( image32F, CV_32F );

    // Multiply with scaling factor to increase contrast
    highCont_1 = image32F * ( 1 + contrastPercentage / 100.0 );

    // Map the float image to [0,1].
    image32F = image32F / 255.0;
    highCont_2 = image32F * ( 1 + contrastPercentage / 100.0 );

    showMat( image, "Original Input Image C" );
    showMat( highCont_1, "incorrect 32 bit high contrast" );
    showMat( highCont_2, "correct 32 bit high contrast", true );

    // Brightness Enhancement
    // I0 = I + b

    int brightnessOffset = 100;

    cv::Mat bright_32F;
    cv::Mat brightHighChannels_32F[ 3 ];

    // Incorrect way.
    cv::Mat tempBright = image_32F.clone( );
    // Add brightness offset to the image directly.
    tempBright = tempBright + brightnessOffset;

    // Correct way.
    // Split the channels.
    cv::split( image_32F, brightHighChannels_32F );
    // Add brightness offset to each channels.
    for ( int i = 0; i < 3; i++ )
    {
        cv::add( brightHighChannels_32F[ i ],
                 brightnessOffset / 255.0,
                 brightHighChannels_32F[ i ] );
    }
    cv::merge( brightHighChannels_32F, 3, bright_32F );

    showMat( image, "Original Input Image B" );
    showMat( tempBright, "Incorrect Brightness Enhanced" );
    showMat( bright_32F, "Correct Brightness Enhanced", true );

    // Performing normalization
    // Add the offset for increasing brightness to the 32 bit image.
    cv::Mat bright_normalized_32F;
    cv::Mat brightHighChannels_norm_32F[ 3 ];
    cv::split( image_32F, brightHighChannels_norm_32F );

    for ( int i = 0; i < 3; i++ )
    {
        cv::add( brightHighChannels_norm_32F[ i ],
             brightnessOffset / 255.0,
             brightHighChannels_norm_32F[ i ] );
    }
    cv::merge( brightHighChannels_norm_32F, 3, bright_normalized_32F );

    // Get maximum pixel intensity.
    double minVal, maxVal;
    cv::minMaxLoc( bright_normalized_32F, &minVal, &maxVal );
    std::cout << "Maximum pixel intensity : " << maxVal;
    bright_normalized_32F = bright_normalized_32F / maxVal;

    showMat( bright_32F, "Brightness Enhanced" );
    showMat( bright_normalized_32F, "Normalized Brightness Enhanced", true );

    return 0;
}