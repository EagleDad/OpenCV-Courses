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

    // Cropping an image
    // Read image
    cv::Mat imageBoy = cv::imread( IMAGES_ROOT + "/boy.jpg" );
    showMat( imageBoy, "Boy", false );

    // Crop out a rectangle
    // x coordinates = 170 to 300
    // y coordinates = 40 to 200
    cv::Mat crop = imageBoy( cv::Range( 40, 200 ), cv::Range( 170, 320 ) );
    showMat( crop, "Boy cropped", true );

    // Copying a region to another
    // First let us create a copy of the original image
    cv::Mat copiedImage = imageBoy.clone( );

    cv::Mat copyRoi = imageBoy( cv::Range( 40, 200 ), cv::Range( 170, 320 ) );

    // Find height and width of the ROI
    int roiHeight = copyRoi.size( ).height;
    int roiWidth = copyRoi.size( ).width;

    // Copy to left of Face
    copyRoi.copyTo( copiedImage( cv::Range( 40, 40 + roiHeight ),
                                 cv::Range( 10, 10 + roiWidth ) ) );
    // Copy to right of Face
    copyRoi.copyTo( copiedImage( cv::Range( 40, 40 + roiHeight ),
                                 cv::Range( 330, 330 + roiWidth ) ) );

    showMat( copiedImage, "Boy copied", true );

    cv::imwrite( RESULTS_ROOT + "/copiedRegions.png", copiedImage );

    // Resizing an image
    int resizeDownWidth = 300;
    int resizeDownHeight = 200;
    cv::Mat resizedDown;
    cv::resize( imageBoy,
                resizedDown,
                cv::Size( resizeDownWidth, resizeDownHeight ),
                0.0,
                0.0,
                cv::INTER_LINEAR );

    // Mess up with the aspect ratio
    int resizeUpWidth = 600;
    int resizeUpHeight = 900;
    cv::Mat resizedUp;
    cv::resize( imageBoy,
                resizedUp,
                cv::Size( resizeUpWidth, resizeUpHeight ),
                0.0,
                0.0,
                cv::INTER_LINEAR );

    showMat( resizedUp, "Boy up", false );
    showMat( resizedDown, "Boy down", true );

    cv::imwrite( RESULTS_ROOT + "/resizedUp.png", resizedUp );
    cv::imwrite( RESULTS_ROOT + "/resizedDown.png", resizedDown );

    // Normally we want to keep the aspect ratio. This can be achieved with th
    // scaling factor Scaling Down the image 1.5 times by specifying both
    // scaling factors
    double scaleUpX = 1.5;
    double scaleUpY = 1.5;

    // Scaling Down the image 0.6 times specifying a single scale factor.
    double scaleDown = 0.6;

    cv::Mat scaledUp, scaledDown;

    cv::resize( imageBoy,
                scaledDown,
                cv::Size( ),
                scaleDown,
                scaleDown,
                cv::INTER_LINEAR );

    cv::resize(
        imageBoy, scaledUp, cv::Size( ), scaleUpX, scaleUpY, cv::INTER_LINEAR );

    showMat( scaledUp, "Boy sup", false );
    showMat( scaledDown, "Boy sdown", true );

    cv::imwrite( RESULTS_ROOT + "/scaledUp.png", scaledUp );
    cv::imwrite( RESULTS_ROOT + "/scaledDown.png", scaledDown );

    std::cout << "Scaled Up Image size = " << scaledUp.size( ) << '\n';
    std::cout << "Scaled Down Image size = " << scaledDown.size( ) << '\n';

    // Creating an image mask
    // Create an empty image of same size as the original
    cv::Mat mask1 = cv::Mat::zeros( imageBoy.size( ), imageBoy.type( ) );
    cv::imwrite( RESULTS_ROOT + "/mask1.png", mask1 );
    showMat( mask1, "Black mask", true );

    /*
    Another way of creating masks is by using some logic. One example is
    using a simple color information. For example, Let us try to find out the
    pixels which are approximately red in color.

    If we want to focus on red pixels, the simplest logic that does the
    trick is:

        * The red channel should have high intensity ( keep the range of pixel
          values from 100 to 255 )
        * The other 2 channels should have low intensity ( keep the range of
           pixel values in Blue and Green channels between 0 to 100) There is a
           nice

    OpenCV function which can do exactly this. We will use the opencv
    function inRange
    */

    cv::Mat mask2;
    cv::inRange(
        imageBoy, cv::Scalar( 0, 0, 150 ), cv::Scalar( 100, 100, 255 ), mask2 );
    cv::imwrite( RESULTS_ROOT + "/mask2.png", mask2 );
    showMat( mask2, "Mask red pixels", true );

    return 0;
}