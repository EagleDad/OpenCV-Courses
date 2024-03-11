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

void readImagesAndTimes( std::vector< cv::Mat >& images,
                         std::vector< float >& times )
{
    int numImages = 4;

    static const float timesArray[] = { 1 / 30.0f, 0.25, 2.5, 15.0 };
    times.assign( timesArray, timesArray + numImages );

    static const char* filenames[] = {
        "img_0.033.jpg", "img_0.25.jpg", "img_2.5.jpg", "img_15.jpg" };

    for ( int i = 0; i < numImages; i++ )
    {
        cv::Mat im = cv::imread( IMAGES_ROOT + "/" + filenames[ i ] );
        images.push_back( im );
    }
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    constexpr float imageScaleFactor = 0.25f;

    std::vector< cv::Mat > images;
    std::vector< float > times;

    //
    // Step 1
    //
    // Read in the different images with their exposure times
    readImagesAndTimes( images, times );

    showMat( images[ 0 ], "Input Image 0", false, imageScaleFactor );
    showMat( images[ 1 ], "Input Image 1", false, imageScaleFactor );
    showMat( images[ 2 ], "Input Image 2", false, imageScaleFactor );
    showMat( images[ 3 ], "Input Image 3", true, imageScaleFactor );

    //
    // Step 2
    //
    // Align the images to avoid ghosting defects
    // There is an overloaded function that also accepts exposure times
    cv::Ptr< cv::AlignMTB > alignMTB = cv::createAlignMTB( );
    alignMTB->process( images, images );

    showMat( images[ 0 ], "Aligned Image 0", false, imageScaleFactor );
    showMat( images[ 1 ], "Aligned Image 1", false, imageScaleFactor );
    showMat( images[ 2 ], "Aligned Image 2", false, imageScaleFactor );
    showMat( images[ 3 ], "Aligned Image 3", true, imageScaleFactor );

    //
    // Step 3
    //
    // Recover the Camera Response Function
    cv::Mat responseDebevec;
    cv::Ptr< cv::CalibrateDebevec > calibrateDebevec =
        cv::createCalibrateDebevec( );
    /* cv::Ptr< cv::CalibrateDebevec > calibrateDebevec =
         cv::createCalibrateRobertson( );*/
    calibrateDebevec->process( images, responseDebevec, times );

    std::vector< cv::Mat > responseChannels( 3 );
    cv::split( responseDebevec, responseChannels );

    // Show as plot using matplot lib
    showMat( responseChannels[ 0 ], "CRF 0", false );
    showMat( responseChannels[ 1 ], "CRF 1", false );
    showMat( responseChannels[ 2 ], "CRF 2", true );

    //
    // Step 4
    //
    // Merge Images
    cv::Mat hdrDebevec;
    cv::Ptr< cv::MergeDebevec > mergeDebevec = cv::createMergeDebevec( );
    mergeDebevec->process( images, hdrDebevec, times, responseDebevec );

    // HDR write might not be active. CHECK
    // cv::imwrite( RESULTS_ROOT + "/hdrDebevec.hdr", hdrDebevec );

    //
    // Step 5
    //
    // Tone mapping
    cv::Mat ldrDrago;
    cv::Ptr< cv::TonemapDrago > tonemapDrago =
        cv::createTonemapDrago( 1.0f, 0.7f );
    tonemapDrago->process( hdrDebevec, ldrDrago );
    ldrDrago = 3 * ldrDrago;

    showMat( ldrDrago, "Drago Tone mapping", true, imageScaleFactor );

    cv::Mat ldrReinhard;
    cv::Ptr< cv::TonemapReinhard > tonemapReinhard =
        cv::createTonemapReinhard( 1.5, 0, 0, 0 );
    tonemapReinhard->process( hdrDebevec, ldrReinhard );

    showMat( ldrReinhard, "Reinhard Tone mapping", true, imageScaleFactor );

    cv::Mat ldrMantiuk;
    cv::Ptr< cv::TonemapMantiuk > tonemapMantiuk =
        cv::createTonemapMantiuk( 2.2f, 0.85f, 1.2f );
    tonemapMantiuk->process( hdrDebevec, ldrMantiuk );
    ldrMantiuk = 3 * ldrMantiuk;

    showMat( ldrMantiuk, "Mantiuk Tone mapping", true, imageScaleFactor );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}