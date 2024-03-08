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

void interp( float* fullRange, float* Curve, float* originalValue, uchar* lut );

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    // Histogram Equalization
    std::string filename = IMAGES_ROOT + "/dark-flowers.jpg";

    cv::Mat img = cv::imread( filename, cv::IMREAD_GRAYSCALE );

    // Equalize histogram
    cv::Mat imEq;
    cv::equalizeHist( img, imEq );

    showMat( img, "Original Image", false );
    showMat( imEq, "Histogram Equalized", true );

    // Display Histogram
    float range[] = { 0.f, 255.0f };

    int32_t levels = 255;
    const float* ranges[] = { range };
    constexpr int32_t hChannels[] = { 0 };
    const int32_t histSize[] = { levels };

    std::vector< float > histogram;

    cv::Mat matHistogram;
    cv::calcHist(
        &img, 1, hChannels, cv::noArray( ), matHistogram, 1, histSize, ranges );

    histogram.resize( static_cast< size_t >( levels ) );

    for ( int32_t row = 0; row < matHistogram.rows; row++ )
    {
        const auto histVal = matHistogram.at< float >( row, 0 );

        histogram[ static_cast< size_t >( row ) ] = histVal;
    }

    showBar( histogram, "Original Histogram", false );

    cv::calcHist( &imEq,
                  1,
                  hChannels,
                  cv::noArray( ),
                  matHistogram,
                  1,
                  histSize,
                  ranges );

    histogram.clear( );
    histogram.resize( static_cast< size_t >( levels ) );

    for ( int32_t row = 0; row < matHistogram.rows; row++ )
    {
        const auto histVal = matHistogram.at< float >( row, 0 );

        histogram[ static_cast< size_t >( row ) ] = histVal;
    }

    showBar( histogram, "Equalized Histogram", true );

    // Histogram Equalization for Color Images

    // Wrong way
    filename = IMAGES_ROOT + "/dark-flowers.jpg";
    img = cv::imread( filename );
    imEq = img.clone( );

    // Perform histogram equalization on each channel separately
    std::vector< cv::Mat > imgChannels( 3 );
    std::vector< cv::Mat > imEqChannels( 3 );

    cv::split( img, imgChannels );
    cv::split( imEq, imEqChannels );

    for ( size_t i = 0; i < 3; i++ )
    {
        cv::equalizeHist( imgChannels[ i ], imEqChannels[ i ] );
    }

    cv::merge( imgChannels, img );
    cv::merge( imEqChannels, imEq );

    showMat( img, "Original Image", false );
    showMat( imEq, "Histogram Equalized", true );

    // Right Way
    // Only the value channel, not changing the color and saturation
    cv::Mat imhsv;

    cv::cvtColor( img, imhsv, cv::COLOR_BGR2HSV );

    std::vector< cv::Mat > imhsvChannels( 3 );

    cv::split( imhsv, imhsvChannels );

    // Perform histogram equalization only on the V channel
    cv::equalizeHist( imhsvChannels[ 2 ], imhsvChannels[ 2 ] );

    cv::merge( imhsvChannels, imhsv );
    // Convert back to BGR format
    cv::cvtColor( imhsv, imEq, cv::COLOR_HSV2BGR );

    showMat( img, "Original Image", false );
    showMat( imEq, "Histogram Equalized", true );

    // Contrast Limited Adaptive Histogram Equalization (CLAHE)

    filename = IMAGES_ROOT + "/night-sky.jpg";
    img = cv::imread( filename );

    cv::cvtColor( img, imhsv, cv::COLOR_BGR2HSV );

    cv::Mat imhsvClahe = imhsv.clone( );

    cv::split( imhsv, imhsvChannels );
    // Perform histogram equalization only on the V channel
    cv::equalizeHist( imhsvChannels[ 2 ], imhsvChannels[ 2 ] );

    cv::Ptr< cv::CLAHE > clahe = cv::createCLAHE( 2.0, cv::Size( 8, 8 ) );

    std::vector< cv::Mat > imhsvClaheChannels( 3 );
    cv::split( imhsvClahe, imhsvClaheChannels );

    clahe->apply( imhsvClaheChannels[ 2 ], imhsvClaheChannels[ 2 ] );

    cv::merge( imhsvChannels, imhsv );
    cv::merge( imhsvClaheChannels, imhsvClahe );
    cv::Mat imEqCLAHE;
    // Convert back to BGR format
    cv::cvtColor( imhsv, imEq, cv::COLOR_HSV2BGR );
    cv::cvtColor( imhsvClahe, imEqCLAHE, cv::COLOR_HSV2BGR );

    showMat( img, "Original Image", false );
    showMat( imEq, "showMat Equalized", false );
    showMat( imEqCLAHE, "CLAHE", true );

    // Color Tone Adjustment using Curves
    // Warming Filter Code
    filename = IMAGES_ROOT + "/girl.jpg";

    cv::Mat Image = cv::imread( filename );

    // Pivot points for X-Coordinates
    float originalValue[] = { 0, 50, 100, 150, 200, 255 };

    // Changed points on Y-axis for each channel
    float rCurve[] = { 0, 80, 150, 190, 220, 255 };
    float bCurve[] = { 0, 20, 40, 75, 150, 255 };

    // Splitting the channels
    std::vector< cv::Mat > channels( 3 );
    cv::split( Image, channels );

    // Create a LookUp Table
    float fullRange[ 256 ];
    int i;
    for ( i = 0; i < 256; i++ )
    {
        fullRange[ i ] = ( float )i;
    }
    cv::Mat lookUpTable( 1, 256, CV_8U );
    uchar* lut = lookUpTable.ptr( );

    cv::Mat bLUT, rLUT;
    // Apply interpolation and create look up table
    interp( fullRange, bCurve, originalValue, lut );

    // Apply mapping and check for underflow/overflow in Blue Channel
    cv::LUT( channels[ 0 ], lookUpTable, bLUT );
    cv::min( bLUT, 255, bLUT );
    cv::max( bLUT, 0, bLUT );

    // Apply interpolation and create look up table
    interp( fullRange, rCurve, originalValue, lut );

    // Apply mapping and check for underflow/overflow in Red Channel
    cv::LUT( channels[ 2 ], lookUpTable, rLUT );
    cv::min( rLUT, 255, rLUT );
    cv::max( rLUT, 0, rLUT );

    cv::Mat output;
    // Merge the channels
    std::vector< cv::Mat > modifiedChannels = { bLUT, channels[ 1 ], rLUT };
    cv::merge( modifiedChannels, output );

    // Display the images
    cv::Mat combined;
    cv::hconcat( Image, output, combined );

    showMat( combined, "Warming filter output", true );

    // Cooling Filter Code
    // Changed points on Y-axis for each channel
    float bCurve2[] = { 0, 80, 150, 190, 220, 255 };
    float rCurve2[] = { 0, 20, 40, 75, 150, 255 };

    // Splitting the channels
    cv::split( Image, channels );

    // Apply interpolation and create look up table
    interp( fullRange, bCurve2, originalValue, lut );

    // Apply mapping and check for underflow/overflow in Blue Channel
    cv::LUT( channels[ 0 ], lookUpTable, bLUT );
    cv::min( bLUT, 255, bLUT );
    cv::max( bLUT, 0, bLUT );

    // Apply interpolation and create look up table
    interp( fullRange, rCurve2, originalValue, lut );

    // Apply mapping and check for underflow/overflow in Red Channel
    cv::LUT( channels[ 2 ], lookUpTable, rLUT );
    cv::min( rLUT, 255, rLUT );
    cv::max( rLUT, 0, rLUT );

    // Merge the channels
    modifiedChannels = { bLUT, channels[ 1 ], rLUT };
    cv::merge( modifiedChannels, output );

    // Display the images
    cv::hconcat( Image, output, combined );

    showMat( combined, "Cooling filter output", true );

    // Clean Up
    cv::destroyAllWindows( );

    return 0;
}

// Piecewise Linear interpolation implemented on a particular Channel
void interp( float* fullRange, float* Curve, float* originalValue, uchar* lut )
{
    int i;
    for ( i = 0; i < 256; i++ )
    {
        int j = 0;
        float a = fullRange[ i ];
        while ( a > originalValue[ j ] )
        {
            j++;
        }
        if ( a == originalValue[ j ] )
        {
            lut[ i ] = static_cast< uint8_t >( Curve[ j ] );
            continue;
        }
        float slope = ( ( float )( Curve[ j ] - Curve[ j - 1 ] ) ) /
                      ( originalValue[ j ] - originalValue[ j - 1 ] );
        float constant = Curve[ j ] - slope * originalValue[ j ];
        lut[ i ] = static_cast< uint8_t >( slope * fullRange[ i ] + constant );
    }
}