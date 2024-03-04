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
    cv::Mat demoImage = cv::Mat::zeros( cv::Size( 10, 10 ), CV_8U );
    std::cout << demoImage << "\n\n";

    // showMat( demoImage, "Input Image" );

    demoImage.at< uchar >( 0, 1 ) = 1;

    demoImage.at< uchar >( 9, 0 ) = 1;

    demoImage.at< uchar >( 8, 9 ) = 1;

    demoImage.at< uchar >( 2, 2 ) = 1;

    demoImage( cv::Range( 5, 8 ), cv::Range( 5, 8 ) ).setTo( 1 );

    std::cout << demoImage << "\n\n";

    cv::Mat element =
        cv::getStructuringElement( cv::MORPH_CROSS, cv::Size( 3, 3 ) );

    std::cout << element << "\n\n";

    int ksize = element.size( ).height;

    int height, width;
    height = demoImage.size( ).height;
    width = demoImage.size( ).width;

    int border = ksize / 2;
    cv::Mat paddedDemoImage = cv::Mat::zeros(
        cv::Size( height + border * 2, width + border * 2 ), CV_8UC1 );

    // Alternative approach to above Mat::zeros code
    copyMakeBorder( paddedDemoImage,
                    paddedDemoImage,
                    border,
                    border,
                    border,
                    border,
                    cv::BORDER_CONSTANT,
                    0 );

    cv::Mat bitOR;

    for ( int h_i = border; h_i < height + border; h_i++ )
    {
        for ( int w_i = border; w_i < width + border; w_i++ )
        {
            if ( demoImage.at< uchar >( h_i - border, w_i - border ) )
            {
                std::cout << "White Pixel Found @ " << h_i << "," << w_i
                          << '\n';
                std::cout << paddedDemoImage(
                                 cv::Range( h_i - border, h_i + border + 1 ),
                                 cv::Range( w_i - border, w_i + border + 1 ) )
                          << '\n';
                std::cout << element << '\n';
                bitwise_or( paddedDemoImage(
                                cv::Range( h_i - border, h_i + border + 1 ),
                                cv::Range( w_i - border, w_i + border + 1 ) ),
                            element,
                            bitOR );
                std::cout << bitOR << '\n';
                std::cout << paddedDemoImage(
                                 cv::Range( h_i - border, h_i + border + 1 ),
                                 cv::Range( w_i - border, w_i + border + 1 ) )
                          << '\n';
                bitOR.copyTo( paddedDemoImage(
                    cv::Range( h_i - border, h_i + border + 1 ),
                    cv::Range( w_i - border, w_i + border + 1 ) ) );
                std::cout << paddedDemoImage << '\n';
            }
        }
    }

    // Crop out the original dimension from the padded output image
    cv::Mat dilatedImage =
        paddedDemoImage( cv::Range( border, border + height ),
                         cv::Range( border, border + width ) );
    showMat( dilatedImage * 255, "Dilated", true, 2 );

    cv::Mat dilatedCrossKernel;
    cv::dilate( demoImage, dilatedCrossKernel, element );
    std::cout << dilatedCrossKernel << "\n";

    showMat( dilatedCrossKernel * 255, "Dilated CV", true, 2 );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}