#include <GUI.h>
#include <macros.h>

IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
IGNORE_WARNINGS_POP

void showMat( const cv::Mat& imageIn, const std::string& szName,
              bool bWait /*= false*/, double scale /*= 1*/ )
{
    cv::namedWindow( szName, cv::WINDOW_NORMAL );

    cv::imshow( szName, imageIn );

    cv::resizeWindow( szName,
                      static_cast< int >( imageIn.cols * scale ),
                      static_cast< int >( imageIn.rows * scale ) );

    cv::waitKey( bWait ? 0 : 100 );
}