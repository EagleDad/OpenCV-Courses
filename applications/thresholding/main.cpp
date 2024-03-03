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

void thresholdingUsingForLoop( cv::Mat src, cv::Mat dst, int thresh,
                               int maxValue )
{
    int height = src.size( ).height;
    int width = src.size( ).width;

    // Loop over rows
    for ( int i = 0; i < height; i++ )
    {
        // Loop over columns
        for ( int j = 0; j < width; j++ )
        {
            if ( src.at< uchar >( i, j ) > thresh )
                dst.at< uchar >( i, j ) = static_cast< uint8_t >( maxValue );
            else
                dst.at< uchar >( i, j ) = 0;
        }
    }
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    // Read an image in grayscale
    const std::string imagePath = IMAGES_ROOT + "/threshold.png";

    cv::Mat src = cv::imread( imagePath, cv::IMREAD_GRAYSCALE );

    // Set threshold and maximum value
    int thresh = 100;
    int maxValue = 255;

    showMat( src, "Input image" );

    cv::Mat dst = src.clone( );
    clock_t t;
    double cpu_time_used;

    t = clock( );
    thresholdingUsingForLoop( src, dst, thresh, maxValue );
    t = clock( ) - t;
    cpu_time_used = ( ( double )t ) / CLOCKS_PER_SEC;
    std::cout << "Time taken = " << cpu_time_used << "\n";

    showMat( dst, "Segmented own" );

    t = clock( );
    cv::threshold( src, dst, thresh, maxValue, cv::THRESH_BINARY );
    t = clock( ) - t;
    cpu_time_used = ( ( double )t ) / CLOCKS_PER_SEC;
    std::cout << "Time taken = " << cpu_time_used << std::endl;

    showMat( dst, "Segmented CV", true );

    clock_t t_loop { }, t_opencv { };
    double time_opencv = 0;
    double time_loops = 0;
    double n_samples = 10.0;

    for ( int i = 0; i < ( int )n_samples; i++ )
    {
        t = clock( );
        thresholdingUsingForLoop( src, dst, thresh, maxValue );
        t = clock( ) - t;
        t_loop += t;

        t = clock( );
        threshold( src, dst, thresh, maxValue, cv::THRESH_BINARY );
        t = clock( ) - t;
        t_opencv += t;
    }

    time_opencv = t_opencv / ( n_samples * CLOCKS_PER_SEC );
    time_loops = t_loop / ( n_samples * CLOCKS_PER_SEC );

    std::cout << "Average time taken by For Loop Code = " << time_loops
              << " seconds\n";
    std::cout << "Average time taken by OpenCV Code = " << time_opencv
              << " seconds\n";

    // Other thresholding types
    thresh = 100;
    maxValue = 150;

    cv::Mat dst_bin;
    threshold( src, dst_bin, thresh, maxValue, cv::THRESH_BINARY );
    showMat( dst_bin, "CV thresh binary", true );

    cv::Mat dst_bin_inv;
    threshold( src, dst_bin_inv, thresh, maxValue, cv::THRESH_BINARY_INV );
    showMat( dst_bin_inv, "CV thresh binary inverse", true );

    cv::Mat dst_trunc;
    threshold( src, dst_trunc, thresh, maxValue, cv::THRESH_TRUNC );
    showMat( dst_trunc, "CV thresh truncate", true );

    cv::Mat dst_to_zero;
    threshold( src, dst_to_zero, thresh, maxValue, cv::THRESH_TOZERO );
    showMat( dst_to_zero, "CV thresh to zero", true );

    cv::Mat dst_to_zero_inv;
    threshold( src, dst_to_zero_inv, thresh, maxValue, cv::THRESH_TOZERO_INV );
    showMat( dst_to_zero_inv, "CV thresh to zero inverse", true );

    // Wait and clean up
    // cv::waitKey( 0 );
    cv::destroyAllWindows( );

    return 0;
}