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

void dilateSelf( )
{
    cv::Mat demoImage = cv::Mat::zeros( cv::Size( 10, 10 ), CV_8U );

    std::cout << demoImage << "\n\n";

    showMat( demoImage * 255, "Image In", true, 8 );

    demoImage.at< uchar >( 0, 1 ) = 1;

    demoImage.at< uchar >( 9, 0 ) = 1;

    demoImage.at< uchar >( 8, 9 ) = 1;

    demoImage.at< uchar >( 2, 2 ) = 1;

    demoImage( cv::Range( 5, 8 ), cv::Range( 5, 8 ) ).setTo( 1 );

    std::cout << demoImage << "\n\n";

    showMat( demoImage * 255, "Image In with blobs", true, 8 );

    cv::Mat element =
        cv::getStructuringElement( cv::MORPH_CROSS, cv::Size( 3, 3 ) );

    std::cout << element << "\n\n";

    showMat( element * 255, "Structure Element", true, 8 );

    int ksize = element.size( ).height;

    int height, width;
    height = demoImage.size( ).height;
    width = demoImage.size( ).width;

    cv::Mat dilatedCrossKernel;
    cv::dilate( demoImage, dilatedCrossKernel, element );
    std::cout << dilatedCrossKernel << "\n\n";

    showMat( dilatedCrossKernel * 255, "Dilated CV", true, 8 );

    int border = ksize / 2;
    cv::Mat paddedDemoImage = cv::Mat::zeros(
        cv::Size( height + border * 2, width + border * 2 ), CV_8UC1 );
    copyMakeBorder( demoImage,
                    paddedDemoImage,
                    border,
                    border,
                    border,
                    border,
                    cv::BORDER_CONSTANT,
                    0 );

    cv::Mat paddedDilatedImage = paddedDemoImage.clone( );
    cv::Mat mask;
    cv::Mat resizedFrame;

    double minVal, maxVal;

    // Create a VideoWriter object
    // Use frame size as 50x50
    ///
    /// YOUR CODE HERE
    ///
    // Define the codec and create VideoWriter object.
    // The output is stored in 'outputChaplin.mp4' file.
    cv::VideoWriter videoDilate( RESULTS_ROOT + "/dilationScratch.avi",
                                 cv::VideoWriter::fourcc( 'F', 'M', 'P', '4' ),
                                 10,
                                 cv::Size( 50, 50 ) );

    cv::Mat bitAnd;

    for ( int h_i = border; h_i < height + border; h_i++ )
    {
        for ( int w_i = border; w_i < width + border; w_i++ )
        {
            cv::bitwise_and(
                paddedDemoImage( cv::Range( h_i - border, h_i + border + 1 ),
                                 cv::Range( w_i - border, w_i + border + 1 ) ),
                element,
                bitAnd );

            cv::minMaxIdx(
                bitAnd, &minVal, &maxVal, nullptr, nullptr, element );

            paddedDilatedImage.at< uchar >( h_i, w_i ) =
                static_cast< uchar >( maxVal );

            showMat( paddedDilatedImage * 255, "Iteration", false, 8 );

            ///
            /// YOUR CODE HERE
            ///
            // Resize output to 50x50 before writing it to the video
            cv::resize( paddedDilatedImage,
                        resizedFrame,
                        cv::Size( 50, 50 ),
                        0,
                        0,
                        cv::INTER_CUBIC );

            ///
            /// YOUR CODE HERE
            ///
            // Convert resizedFrame to BGR before writing
            cv::Mat imageVideoOut;
            cv::Mat channels[ 3 ];
            channels[ 0 ] = resizedFrame * 255;
            channels[ 1 ] = resizedFrame * 255;
            channels[ 2 ] = resizedFrame * 255;

            cv::merge( channels, 3, imageVideoOut );

            ///
            /// YOUR CODE HERE
            ///
            // Write the frame into the file 'dilationScratch.avi'
            videoDilate.write( imageVideoOut );
        }
    }

    // Crop out the original dimension from the padded output image
    cv::Mat dilatedImageOwn =
        paddedDilatedImage( cv::Range( border, border + height ),
                            cv::Range( border, border + width ) );
    showMat( dilatedImageOwn * 255, "Dilated OWN", true, 8 );

    // Check if images are equal
    cv::Mat sub;
    cv::subtract( dilatedImageOwn, dilatedCrossKernel, sub );
    auto nonZero = cv::countNonZero( sub );

    if ( nonZero == 0 )
    {
        std::cout << "Images are equal\n";
    }
    else
    {
        std::cout << "Images not equal\n";
    }

    videoDilate.release( );
}

void erodeSelf( )
{
    cv::Mat demoImage = cv::Mat::zeros( cv::Size( 10, 10 ), CV_8U );

    std::cout << demoImage << "\n\n";

    showMat( demoImage * 255, "Image In", true, 4 );

    demoImage.at< uchar >( 0, 1 ) = 1;

    demoImage.at< uchar >( 9, 0 ) = 1;

    demoImage.at< uchar >( 8, 9 ) = 1;

    demoImage.at< uchar >( 2, 2 ) = 1;

    demoImage( cv::Range( 5, 8 ), cv::Range( 5, 8 ) ).setTo( 1 );

    std::cout << demoImage << "\n\n";

    showMat( demoImage * 255, "Image In with blobs", true, 4 );

    cv::Mat element =
        cv::getStructuringElement( cv::MORPH_CROSS, cv::Size( 3, 3 ) );

    std::cout << element << "\n\n";

    showMat( element * 255, "Structure Element", true, 4 );

    int ksize = element.size( ).height;

    int height, width;
    height = demoImage.size( ).height;
    width = demoImage.size( ).width;

    cv::Mat ErodedCrossKernel;
    cv::erode( demoImage, ErodedCrossKernel, element );
    std::cout << ErodedCrossKernel << "\n\n";

    showMat( ErodedCrossKernel * 255, "Eroded CV", true, 8 );

    int border = ksize / 2;
    cv::Mat paddedDemoImage = cv::Mat::zeros(
        cv::Size( height + border * 2, width + border * 2 ), CV_8UC1 );
    copyMakeBorder( demoImage,
                    paddedDemoImage,
                    border,
                    border,
                    border,
                    border,
                    cv::BORDER_CONSTANT,
                    0 );

    cv::Mat paddedErodedImage = paddedDemoImage.clone( );
    cv::Mat mask;
    cv::Mat resizedFrame;

    double minVal, maxVal;

    // Create a VideoWriter object
    // Use frame size as 50x50
    ///
    /// YOUR CODE HERE
    ///
    // Define the codec and create VideoWriter object.
    // The output is stored in 'outputChaplin.mp4' file.
    cv::VideoWriter out( RESULTS_ROOT + "/erosionScratch.avi",
                         cv::VideoWriter::fourcc( 'F', 'M', 'P', '4' ),
                         10,
                         cv::Size( 50, 50 ) );

    cv::Mat bitAnd;

    for ( int h_i = border; h_i < height + border; h_i++ )
    {
        for ( int w_i = border; w_i < width + border; w_i++ )
        {
            cv::bitwise_and(
                paddedDemoImage( cv::Range( h_i - border, h_i + border + 1 ),
                                 cv::Range( w_i - border, w_i + border + 1 ) ),
                element,
                bitAnd );

            cv::minMaxIdx(
                bitAnd, &minVal, &maxVal, nullptr, nullptr, element );

            paddedErodedImage.at< uchar >( h_i, w_i ) =
                static_cast< uchar >( minVal );

            ///
            /// YOUR CODE HERE
            ///
            // Resize output to 50x50 before writing it to the video
            cv::resize( paddedErodedImage,
                        resizedFrame,
                        cv::Size( 50, 50 ),
                        0,
                        0,
                        cv::INTER_CUBIC );

            ///
            /// YOUR CODE HERE
            ///
            // Convert resizedFrame to BGR before writing
            cv::Mat imageVideoOut;
            cv::Mat channels[ 3 ];
            channels[ 0 ] = resizedFrame * 255;
            channels[ 1 ] = resizedFrame * 255;
            channels[ 2 ] = resizedFrame * 255;

            cv::merge( channels, 3, imageVideoOut );

            showMat( paddedErodedImage * 255, "Iteration", false, 8 );

            ///
            /// YOUR CODE HERE
            ///

            // Write the frame into the file 'erosionScratch.avi'
            out.write( imageVideoOut );
        }
    }

    // Crop out the original dimension from the padded output image
    cv::Mat erodedImageOwn =
        paddedErodedImage( cv::Range( border, border + height ),
                           cv::Range( border, border + width ) );
    showMat( erodedImageOwn * 255, "Erodes OWN", true, 8 );

    // Check if images are equal
    cv::Mat sub;
    cv::subtract( erodedImageOwn, ErodedCrossKernel, sub );
    auto nonZero = cv::countNonZero( sub );

    if ( nonZero == 0 )
    {
        std::cout << "Images are equal\n";
    }
    else
    {
        std::cout << "Images not equal\n";
    }

    out.release( );
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    dilateSelf( );

    erodeSelf( );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}