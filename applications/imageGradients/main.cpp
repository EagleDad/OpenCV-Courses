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
    // Sobel filter
    std::string filename = IMAGES_ROOT + "/truth.png";

    // Read the image
    cv::Mat image = cv::imread( filename, cv::IMREAD_GRAYSCALE );

    cv::Mat sobelx, sobely;

    // Apply sobel filter with only x gradient
    cv::Sobel( image, sobelx, CV_32F, 1, 0 );

    // Apply sobel filter with only y gradient
    cv::Sobel( image, sobely, CV_32F, 0, 1 );

    // Normalize image for display
    // The function normalized between alpha and beta, here 0 and 1
    cv::normalize( sobelx, sobelx, 0, 1, cv::NORM_MINMAX );
    cv::normalize( sobely, sobely, 0, 1, cv::NORM_MINMAX );

    showMat( sobelx /** 255*/, "X Gradients", false );

    showMat( sobely /** 255*/, "Y Gradients", true );

    // Second Order Derivative Filters
    // Read the image
    image = imread( IMAGES_ROOT + "/sample.jpg", cv::IMREAD_GRAYSCALE );

    cv::Mat LOG, img1;
    int kernelSize = 3;

    // Applying laplacian
    cv::GaussianBlur( image, img1, cv::Size( 3, 3 ), 0, 0 );
    cv::Laplacian( img1, LOG, CV_32F, kernelSize, 1, 0 );

    // Normalize images
    cv::normalize( LOG, LOG, 0, 1, cv::NORM_MINMAX, CV_32F );

    showMat( LOG /** 255*/, "Laplacian", true );

    // Application - Sharpening Filter

    /*
     * Fortunately, it is much easier for us to achieve unsharp masking
     * digitally.
     *
     *   Step 1: Blur the image to smooth out texture. The blurred image
     *      contains low frequency information of the original image. Let  I be
     *      the original image and Ib be the blurred image.
     *
     *  Step 2: Obtain the high frequency information of the original image by
     *      subtracting the blurred image from the original image.
     *
     *  Step 3: Now, put back the high frequency information back in the image
     *      and control the amount using a parameter. The final sharpened image
     *      is therefore,
     *
     *  Is = I + alpha(I - Ib)
     **/

    // Read the image
    image = cv::imread( IMAGES_ROOT + "/sample.jpg" );

    // Sharpen filter can be obtained:
    //
    // | 0 0 0 |   | 0  -1  0  |   | 0  -1  0  |
    // | 0 1 0 | + | -1  4  -1 | = | -1  5  -1 |
    // | 0 0 0 |   | 0  -1  0  |   | 0  -1  0  |

    cv::Mat sharpen =
        ( cv::Mat_< int >( 3, 3 ) << 0, -1, 0, -1, 5, -1, 0, -1, 0 );

    cv::Mat sharpenOutput;
    cv::filter2D( image, sharpenOutput, -1, sharpen );

    showMat( image, "Original Image", false );
    showMat( sharpenOutput, "Sharpening Result", true );

    // Canny Edge Detection

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}