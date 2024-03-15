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

std::string windowNameSrc = "Source Image";
std::string windowNameTgt = "Target Image";
std::string windowNameAd = "Advertisement";

cv::Mat orgImage;
cv::Mat sourceImage;
cv::Mat targetImage;
cv::Mat replacementImage;

std::vector< cv::Point2i > pts_src;
std::vector< cv::Point2i > pts_dst;

void updateView( )
{
    cv::imshow( windowNameSrc, sourceImage );
    cv::imshow( windowNameTgt, targetImage );
    cv::imshow( windowNameAd, replacementImage );
}

void onMouse( int action, int x, int y, int flags, void* userdata )
{
    std::ignore = action;
    std::ignore = x;
    std::ignore = y;
    std::ignore = flags;
    std::ignore = userdata;

    if ( action == cv::EVENT_LBUTTONDOWN )
    {
        // Collect four points and then transform the advertisement to the
        // target image
        cv::Point2i loc( x, y );

        cv::circle( sourceImage, loc, 5, cv::Scalar( 0, 255, 0 ), 5, -1 );

        pts_dst.emplace_back( loc );

        if ( pts_dst.size( ) == 4 )
        {
            targetImage = orgImage.clone( );
            updateView( );

            pts_src.emplace_back( 0, 0 );
            pts_src.emplace_back( replacementImage.cols, 0 );
            pts_src.emplace_back( replacementImage.cols,
                                  replacementImage.rows );
            pts_src.emplace_back( 0, replacementImage.rows );

            const cv::Mat homography = cv::findHomography( pts_src, pts_dst );

            cv::Mat tmp = targetImage.clone( );

            cv::warpPerspective(
                replacementImage, tmp, homography, tmp.size( ) );

            // Black out polygonal area in destination image.
            cv::fillConvexPoly(
                targetImage, pts_dst.data( ), 4, cv::Scalar( 0 ), cv::LINE_AA );

            // Add warped source image to destination image.
            targetImage = targetImage + tmp;

            sourceImage = orgImage.clone( );

            updateView( );

            // Cleanup
            pts_dst.clear( );
            pts_src.clear( );
        }

        updateView( );
    }
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    int key { };

    sourceImage =
        cv::imread( IMAGES_ROOT + "/times-square.jpg", cv::IMREAD_COLOR );

    orgImage = sourceImage.clone( );

    targetImage =
        cv::imread( IMAGES_ROOT + "/times-square.jpg", cv::IMREAD_COLOR );

    replacementImage =
        cv::imread( IMAGES_ROOT + "/first-image.jpg", cv::IMREAD_COLOR );

    cv::namedWindow( windowNameSrc );
    cv::namedWindow( windowNameTgt );
    cv::namedWindow( windowNameAd );

    cv::imshow( windowNameSrc, sourceImage );
    cv::imshow( windowNameTgt, targetImage );
    cv::imshow( windowNameAd, replacementImage );

    // highgui function called when mouse events occur
    cv::setMouseCallback( windowNameSrc, onMouse );

    while ( key != 27 )
    {
        key = cv::waitKey( 20 ) & 0xFF;
    }

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}
