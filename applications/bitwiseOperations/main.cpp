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
    // Load the Face Image
    std::string faceImagePath = IMAGES_ROOT + "/musk.jpg ";
    cv::Mat faceImage = cv::imread( faceImagePath );

    // Make a copy
    cv::Mat faceWithGlassesBitwise = faceImage.clone( );

    // Load the Sunglass image with Alpha channel
    std::string glassimagePath = IMAGES_ROOT + "/sunglass.png";
    // Read the image
    cv::Mat glassPNG = cv::imread( glassimagePath, -1 );
    // Resize the image to fit over the eye region
    cv::resize( glassPNG, glassPNG, cv::Size( 300, 100 ) );
    std::cout << "image Dimension = " << glassPNG.size( ) << '\n';

    // Separate the Color and alpha channels
    cv::Mat glassRGBAChannels[ 4 ];
    cv::Mat glassRGBChannels[ 3 ];
    split( glassPNG, glassRGBAChannels );
    for ( int i = 0; i < 3; i++ )
    {
        // Copy R,G,B channel from RGBA to RGB
        glassRGBChannels[ i ] = glassRGBAChannels[ i ];
    }
    cv::Mat glassBGR, glassMask1;
    // Prepare BGR Image
    cv::merge( glassRGBChannels, 3, glassBGR );
    // Alpha channel is the 4th channel in RGBA Image
    glassMask1 = glassRGBAChannels[ 3 ];
    cv::imwrite( RESULTS_ROOT + "/sunglassRGB.png", glassBGR );
    cv::imwrite( RESULTS_ROOT + "/sunglassAlpha.png", glassMask1 );

    showMat( glassBGR, "sunglassRGB" );
    showMat( glassMask1, "sunglassAlpha" );

    // Get the eye region from the face image
    cv::Mat eyeROI =
        faceWithGlassesBitwise( cv::Range( 150, 250 ), cv::Range( 140, 440 ) );

    // Make the dimensions of the mask same as the input image.
    // Since Face Image is a 3-channel image, we create a 3 channel image for
    // the mask
    cv::Mat glassMask;
    cv::Mat glassMaskChannels[] = { glassMask1, glassMask1, glassMask1 };
    cv::merge( glassMaskChannels, 3, glassMask );

    // Use the mask to create the masked eye region
    cv::Mat eye;
    cv::Mat glassMaskNOT;
    cv::bitwise_not( glassMask1, glassMaskNOT );

    cv::Mat eyeChannels[ 3 ];
    cv::Mat eyeROIChannels[ 3 ];
    cv::Mat maskedGlass;
    cv::Mat eyeRoiFinal;

    cv::split( eyeROI, eyeROIChannels );

    for ( int i = 0; i < 3; i++ )
    {
        cv::bitwise_and( eyeROIChannels[ i ], glassMaskNOT, eyeChannels[ i ] );
    }

    cv::merge( eyeChannels, 3, eye );

    cv::imwrite( RESULTS_ROOT + "/glassMaskNOT.png", glassMaskNOT );

    cv::Mat glassMaskNOTChannels[] = {
        glassMaskNOT, glassMaskNOT, glassMaskNOT };
    cv::Mat glassMaskNOTMerged;
    // The following code is an alternative way of merging the 3 channels
    // Another way has been shown in the code above (Mat glassMaskNOTChannels[]
    // = ...)
    cv::merge( glassMaskNOTChannels, 3, glassMaskNOTMerged );

    cv::bitwise_and( eyeROI, glassMaskNOTMerged, eye );
    // Use the mask to create the masked sunglass region
    cv::Mat sunglass;
    cv::Mat sunglassChannels[ 3 ];
    cv::Mat glassBGRChannels[ 3 ];

    cv::split( glassBGR, glassBGRChannels );

    for ( int i = 0; i < 3; i++ )
    {
        cv::bitwise_and(
            glassBGRChannels[ i ], glassMask1, sunglassChannels[ i ] );
    }

    cv::merge( sunglassChannels, 3, sunglass );
    // The following code is an alternative to the bitwise_and
    // approach we have followed above
    cv::multiply( glassBGR, glassMask, maskedGlass );

    // Combine the Sunglass in the Eye Region to get the augmented image
    cv::bitwise_or( eye, sunglass, eyeRoiFinal );

    cv::imwrite( RESULTS_ROOT + "/maskedEyeRegionBitwise.png", eye );
    cv::imwrite( RESULTS_ROOT + "/maskedSunglassRegionBitwise.png", sunglass );
    cv::imwrite( RESULTS_ROOT + "/augmentedEyeAndSunglassBitwise.png",
                 eyeRoiFinal );

    showMat( eye, "Eye" );
    showMat( sunglass, "Sunglass" );
    showMat( eyeRoiFinal, "EyeRoiFinal" );

    // Replace the eye ROI with the output from the previous section
    eyeRoiFinal.copyTo( eyeROI );

    cv::imwrite( RESULTS_ROOT + "/withSunglassesBitwise.png",
                 faceWithGlassesBitwise );

    showMat( faceWithGlassesBitwise, "faceWithGlassesBitwise", true );

    return 0;
}