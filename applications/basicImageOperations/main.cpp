#include <GUI.h>
#include <macros.h>

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
IGNORE_WARNINGS_POP

// STD includes
#include <iostream>

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    // Create an empty image matrix
    cv::Mat emptyMat = cv::Mat( 100, 200, CV_8UC1, cv::Scalar::all( 0 ) );

    showMat( emptyMat, "Empty Mat" );

    // Set all white
    emptyMat.setTo( cv::Scalar::all( 255 ) );

    showMat( emptyMat, "White Mat" );

    // Create Mat same size
    cv::Mat emptyOrg =
        cv::Mat( emptyMat.size( ), emptyMat.type( ), cv::Scalar::all( 128 ) );

    showMat( emptyOrg, "Gray Mat", true );
    return 0;
}