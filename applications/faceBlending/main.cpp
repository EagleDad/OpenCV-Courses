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
    // Read images
    cv::Mat dst = cv::imread( IMAGES_ROOT + "/trump.jpg" );

    cv::Mat src = cv::imread( IMAGES_ROOT + "/obama.jpg" );

    cv::Mat src_mask =
        imread( IMAGES_ROOT + "/obama-mask.jpg", cv::IMREAD_GRAYSCALE );

    showMat( dst, "Trump Destination", false );
    showMat( src, "Obama Source", false );
    showMat( src_mask, "Obama Mask", true );

    /*
     * Simple Alpha Blending with Mask
     *   The lighting in the images is very different
     *   The skin tones are very different
     *   The blend will look ridiculous
     */
    cv::Mat alpha;
    cv::cvtColor( src_mask, alpha, cv::COLOR_GRAY2BGR );
    alpha.convertTo( alpha, CV_32FC3 );
    alpha = alpha / 255.0;

    src.convertTo( src, CV_32FC3 );
    dst.convertTo( dst, CV_32FC3 );

    std::vector< cv::Mat > srcChannels( 3 ), dstChannels( 3 ),
        alphaChannels( 3 ), outputBlendChannels( 3 );
    cv::split( src, srcChannels );
    cv::split( dst, dstChannels );
    cv::split( alpha, alphaChannels );

    for ( size_t i = 0; i < 3; i++ )
    {
        outputBlendChannels[ i ] =
            srcChannels[ i ].mul( alphaChannels[ i ] ) +
            dstChannels[ i ].mul( 1 - alphaChannels[ i ] );
    }

    cv::Mat output_blend;
    cv::merge( outputBlendChannels, output_blend );

    output_blend.convertTo( output_blend, CV_8UC3 );

    showMat( output_blend, "Obama blended Trump", true );

    // Find blob centroid
    cv::Mat src_mask_bin;
    cv::threshold( src_mask, src_mask_bin, 128, 255, cv::THRESH_BINARY );

    cv::Moments m = cv::moments( src_mask_bin );

    cv::Point center( static_cast< int >( m.m01 / m.m00 ),
                      static_cast< int >( m.m10 / m.m00 ) );

    // Clone seamlessly
    cv::Mat output_clone;
    src.convertTo( src, CV_8UC3 );
    dst.convertTo( dst, CV_8UC3 );
    src_mask.convertTo( src_mask, CV_8UC3 );
    cv::seamlessClone( src, dst, src_mask, center, output_clone, cv::NORMAL_CLONE );
    output_clone.convertTo( output_clone, CV_8UC3 );

    showMat( output_clone, "Obama cloned Trump", true );


    // Clean up
    cv::destroyAllWindows( );

    return 0;
}