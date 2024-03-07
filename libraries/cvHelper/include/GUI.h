#pragma once

#include <cvHelper/export.h>

// STD includes
#include <string>
#include <vector>

#include <macros.h>

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
IGNORE_WARNINGS_POP

namespace cv
{
class Mat;
}

CVHELPER_EXPORT
void showMat( const cv::Mat& imageIn, const std::string& szName,
              bool bWait = false, double scale = 1, int32_t waitTime = 25 );

template < typename ValueType >
void showBar( std::vector< ValueType > profile, const std::string& szName,
              bool bWait = false, double scale = 1,
              cv::Scalar color = cv::Scalar( 0, 0, 255 ) )
{
    // Use 256 extra pixel for legend
    const auto matWidth = static_cast< int32_t >( profile.size( ) + 256 );
    const auto dataWidth = static_cast< int32_t >( profile.size( ) );
    auto matShow = cv::Mat( 256, matWidth, CV_8UC3, cv::Scalar::all( 0x00 ) );

    const auto [ min, max ] =
        std::minmax_element( profile.begin( ), profile.end( ) );

    const double minVal = *min;
    const double maxVal = *max;

    double c1 = minVal * -1.0;
    double c2 = 255 / ( maxVal - minVal );

    for ( int32_t x = 0; x < dataWidth; x++ )
    {
        const double newVal =
            ( profile[ static_cast< size_t >( x ) ] + c1 ) * c2;
        const auto p1 = cv::Point( x, matShow.rows );
        const auto p2 =
            cv::Point( x, matShow.rows - static_cast< int32_t >( newVal ) );
        cv::line( matShow, p1, p2, color );
    }

    const auto yValueZero =
        matShow.rows - static_cast< int32_t >( std::round( ( 0 + c1 ) * c2 ) );
    const auto p1 = cv::Point( 0, yValueZero );
    const auto p2 = cv::Point( dataWidth, yValueZero );

    cv::line( matShow, p1, p2, cv::Scalar( 255, 255, 255 ) );

    const std::string textMax = "Max: " + std::to_string( *max );
    const std::string textMin = "Min: " + std::to_string( *min );

    putText( matShow,
             textMax,
             { dataWidth, 15 },
             cv::FONT_HERSHEY_PLAIN,
             1,
             cv::Scalar( 0, 255, 0 ) );

    putText( matShow,
             textMin,
             { dataWidth, 30 },
             cv::FONT_HERSHEY_PLAIN,
             1,
             cv::Scalar( 0, 255, 0 ) );

    showMat( matShow, szName, bWait, scale );
}
