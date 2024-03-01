#pragma once

#include <cvHelper/export.h>

// STD includes
#include <string>

namespace  cv
{
class Mat;
}

CVHELPER_EXPORT
void showMat( const cv::Mat& imageIn, const std::string& szName,
              bool bWait = false, double scale = 1 );
