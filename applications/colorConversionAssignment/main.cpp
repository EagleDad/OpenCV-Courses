#include <GUI.h>
#include <macros.h>

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
IGNORE_WARNINGS_POP

// STD includes
#include <iostream>

const std::string IMAGES_ROOT = "C:/images";
const std::string RESULTS_ROOT = "C:/images/results";

cv::Mat convertBGRtoGray( cv::Mat image )
{
    ///
    /// YOUR CODE HERE
    ///
    cv::Mat imageGray( image.size( ), CV_8UC1 );

    for ( int32_t y = 0; y < image.size( ).height; y++ )
    {
        const auto srcPtr = image.ptr< cv::Vec3b >( y );
        const auto dstPtr = imageGray.ptr< uint8_t >( y );

        for ( int32_t x = 0; x < image.size( ).width; x++ )
        {
            const auto Y = srcPtr[ x ][ 0 ] * 0.114 + srcPtr[ x ][ 1 ] * 0.587 +
                           srcPtr[ x ][ 2 ] * 0.299;

            dstPtr[ x ] = cv::saturate_cast< uint8_t >( Y );
        }
    }

    return imageGray;
}

cv::Mat convertBGRtoHSV( cv::Mat image )
{
    ///
    /// YOUR CODE HERE
    ///
    cv::Mat imageHSV( image.size( ), CV_8UC3 );

    auto isZero = []( double value )
    {
        return std::fabs( value ) <= std::numeric_limits< double >::epsilon( );
    };

    auto isEqual = []( double a, double b )
    {
        return std::fabs( a - b ) <= std::numeric_limits< double >::epsilon( );
    };

    for ( int32_t y = 0; y < image.size( ).height; y++ )
    {
        const auto srcPtr = image.ptr< cv::Vec3b >( y );
        const auto dstPtr = imageHSV.ptr< cv::Vec3b >( y );

        for ( int32_t x = 0; x < image.size( ).width; x++ )
        {
            const auto R = static_cast< double >( srcPtr[ x ][ 2 ] ) / 255.0;
            const auto G = static_cast< double >( srcPtr[ x ][ 1 ] ) / 255.0;
            const auto B = static_cast< double >( srcPtr[ x ][ 0 ] ) / 255.0;

            const auto Vmax = std::max( R, std::max( G, B ) );
            const auto Vmin = std::min( R, std::min( G, B ) );

            const auto VDelta = Vmax - Vmin;

            double S { };

            if ( ! isZero( Vmax ) )
            {
                S = VDelta / Vmax;
            }

            double H { };

            if ( isEqual( Vmax, R ) )
            {
                H = 60.0 * ( G - B ) / VDelta;
            }
            else if ( isEqual( Vmax, G ) )
            {
                H = 120.0 + 60.0 * ( B - R ) / VDelta;
            }
            else if ( isEqual( Vmax, B ) )
            {
                H = 240.0 + 60.0 * ( R - G ) / VDelta;
            }
            else if ( isZero( R ) || isZero( G ) || isZero( B ) )
            {
                H = 0.0;
            }

            if ( H < 0.0 )
            {
                H += 360.0;
            }

            H /= 2.0;

            dstPtr[ x ] = cv::Vec3b(
                static_cast< uint8_t >( std::round( H ) ),
                static_cast< uint8_t >( std::round( S * 255.0 ) ),
                static_cast< uint8_t >( std::round( Vmax * 255.0 ) ) );
        }
    }

    return imageHSV;
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    cv::Mat img = cv::imread( IMAGES_ROOT + "/sample.jpg" );
    showMat( img, "Sample Image", false );

    auto imageGrayOwn = convertBGRtoGray( img );
    showMat( imageGrayOwn, "Sample Image Gray Own", false );

    cv::Mat imageGrayCv;
    cv::cvtColor( img, imageGrayCv, cv::COLOR_BGR2GRAY );

    cv::Mat subtracted;
    cv::subtract( imageGrayOwn, imageGrayCv, subtracted );
    auto diff = cv::countNonZero( subtracted );
    std::cout << "Different pixels: " << diff << "\n";

    showMat( imageGrayCv, "Sample Image Gray CV", true );

    cv::Mat imageHsvOwn = convertBGRtoHSV( img );

    cv::Mat imageHsvCv;
    cv::cvtColor( img, imageHsvCv, cv::COLOR_BGR2HSV );

    cv::Mat channelsCv[ 3 ];
    cv::Mat channelsOwn[ 3 ];
    cv::split( imageHsvOwn, channelsOwn );
    cv::split( imageHsvCv, channelsCv );

    for ( int i = 0; i < 3; i++ )
    {
        int count { };
        cv::subtract( channelsOwn[ i ], channelsCv[ i ], subtracted );
        diff = cv::countNonZero( subtracted );
        std::cout << "Different pixels: " << diff << "\n";

        for ( int32_t y = 0; y < channelsOwn[ i ].size( ).height; y++ )
        {
            const auto srcPtr0 = channelsOwn[ i ].ptr< uint8_t >( y );
            const auto srcPtr1 = channelsCv[ i ].ptr< uint8_t >( y );

            for ( int32_t x = 0; x < channelsOwn[ i ].size( ).width; x++ )
            {
                if ( srcPtr0[ x ] != srcPtr1[ x ] )
                {
                    count++;
                }
            }
        }
        std::cout << "Different Pixels at channel " << i << ": " << count
                  << "\n";
    }

    cv::Mat diffHSV;
    cv::absdiff( imageHsvOwn, imageHsvCv, diffHSV );

    showMat( diffHSV, "Diff Image HSV", true );

    cv::destroyAllWindows( );

    return 0;
}