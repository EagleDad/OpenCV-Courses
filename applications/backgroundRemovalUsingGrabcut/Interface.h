#pragma once

#include <macros.h>

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
IGNORE_WARNINGS_POP



// Convention of defining color in opencv is BGR
const cv::Scalar RED = cv::Scalar( 0, 0, 255 );
const cv::Scalar PINK = cv::Scalar( 230, 130, 255 );
const cv::Scalar BLUE = cv::Scalar( 255, 0, 0 );
const cv::Scalar LIGHTBLUE = cv::Scalar( 255, 255, 160 );
const cv::Scalar GREEN = cv::Scalar( 0, 255, 0 );

class Interface
{
public:
    enum
    {
        NOT_SET = 0,
        IN_PROCESS = 1,
        SET = 2
    };

    static const int radius = 2;
    static const int thickness = -1;

    void reset( );
    void setImageAndWinName( const cv::Mat& _image,
                             const std::string& _winName );
    void showImage( ) const;
    void mouseClick( int event, int x, int y, int flags, void* param );
    int nextIter( );
    int getIterCount( ) const { return iterCount; }

private:
    void setRectInMask( );
    void setLblsInMask( int flags, cv::Point p, bool isPr );

    const std::string* winName;
    const cv::Mat* image;
    cv::Mat mask;
    cv::Mat bgdModel, fgdModel;

    uchar rectState, lblsState, prLblsState;
    bool isInitialized;

    cv::Rect rect;
    std::vector< cv::Point > FG_PIXEL, BG_PIXEL, prFG_PIXEL, prBG_PIXEL;
    int iterCount;
};

inline Interface app;

inline int global_value;

// On every area selection, changes are updated through binmask
void getBinMask( const cv::Mat& comMask, cv::Mat& binMask );

void on_mouse( int event, int x, int y, int flags, void* param );
