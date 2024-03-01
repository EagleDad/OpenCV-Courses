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
    constexpr bool naive = false;
    // Path to image
    const std::string faceImagePath = IMAGES_ROOT + "/musk.jpg";

    // Read the image
    cv::Mat faceImage = cv::imread( faceImagePath );
    if ( naive )
    {
        faceImage.convertTo( faceImage, CV_32FC3 );
        faceImage = faceImage / 255.0;
    }

    // Load the Sunglass image with Alpha channel
    // (http://pluspng.com/sunglass-png-1104.html)
    std::string glassimagePath = IMAGES_ROOT + "/sunglass.png";

    // Read the image
    cv::Mat glassPNG = cv::imread( glassimagePath, -1 );
    if ( naive )
    {
        glassPNG.convertTo( glassPNG, CV_32F );
        glassPNG = glassPNG / 255.0;
    }

    // Resize the image to fit over the eye region
    // cv::resize( glassPNG, glassPNG, cv::Size( ), 0.5, 0.5 );
    //  Resize the image to fit over the eye region
    cv::resize( glassPNG, glassPNG, cv::Size( 300, 100 ) );

    std::cout << "Image Dimension = " << glassPNG.size( ) << '\n';
    std::cout << "Number of channels = " << glassPNG.channels( ) << '\n';

    // int glassHeight = glassPNG.size( ).height;
    // int glassWidth = glassPNG.size( ).width;

    // Separate the Color and alpha channels
    cv::Mat glassRGBAChannels[ 4 ];
    cv::Mat glassRGBChannels[ 3 ];
    cv::split( glassPNG, glassRGBAChannels );

    for ( int i = 0; i < 3; i++ )
    {
        // Copy R,G,B channel from RGBA to RGB
        glassRGBChannels[ i ] = glassRGBAChannels[ i ];
    }

    cv::Mat elonSplit[ 3 ];
    cv::split( faceImage, elonSplit );

    for ( int i = 0; i < 3; i++ )
    {
        showMat( elonSplit[ i ], "Elon " + std::to_string( i ), false );
        cv::Mat channelSeg;
        cv::threshold( elonSplit[ i ],
                       channelSeg,
                       180,
                       255,
                       cv::THRESH_BINARY/* | cv::THRESH_OTSU*/ );
        showMat( channelSeg, "Elon seg" + std::to_string( i ), false );
    }

    cv::Mat glassBGR, glassMask1;
    // Prepare BGR Image
    cv::merge( glassRGBChannels, 3, glassBGR );
    // Alpha channel is the 4th channel in RGBA Image
    glassMask1 = glassRGBAChannels[ 3 ];

    // show the  face image
    showMat( faceImage, "Elon", false );

    // sow galsses image
    showMat( glassBGR, "Sunglass Color channels", false );

    showMat( glassMask1 * 255, "Sunglass Alpha channel", false );

    if ( naive )
    {
        // Make a copy
        cv::Mat faceWithGlassesNaive = faceImage.clone( );
        cv::Mat roiFace = faceWithGlassesNaive( cv::Range( 150, 250 ),
                                                cv::Range( 140, 440 ) );
        // Replace the eye region with the sunglass image
        glassBGR.copyTo( roiFace );

        showMat( faceWithGlassesNaive, "Face with glasses", true );
    }
    else
    {
        // Make the dimensions of the mask same as the input image.
        // Since Face Image is a 3-channel image, we create a 3 channel image
        // for the mask
        cv::Mat glassMask;
        cv::Mat glassMaskChannels[] = { glassMask1, glassMask1, glassMask1 };
        cv::merge( glassMaskChannels, 3, glassMask );

        // Make the values [0,1] since we are using arithmetic operations
        glassMask = glassMask / 255;

        // Make a copy
        cv::Mat faceWithGlassesArithmetic = faceImage.clone( );

        // Get the eye region from the face image
        cv::Mat eyeROI = faceWithGlassesArithmetic( cv::Range( 150, 250 ),
                                                    cv::Range( 140, 440 ) );

        cv::Mat eyeROIChannels[ 3 ];
        cv::split( eyeROI, eyeROIChannels );
        cv::Mat maskedEyeChannels[ 3 ];
        cv::Mat maskedEye;

        for ( int i = 0; i < 3; i++ )
        {
            // Use the mask to create the masked eye region
            cv::multiply( eyeROIChannels[ i ],
                          ( 1 - glassMaskChannels[ i ] ),
                          maskedEyeChannels[ i ] );
        }

        cv::merge( maskedEyeChannels, 3, maskedEye );

        cv::Mat maskedGlass;
        // Use the mask to create the masked sunglass region
        cv::multiply( glassBGR, glassMask, maskedGlass );

        cv::Mat eyeRoiFinal;
        cv::Mat eyeRoiTemp;
        cv::Mat eyeRoiParcial;
        // Combine the Sunglass in the Eye Region to get the augmented image
        // cv::add( maskedEye, maskedGlass, eyeRoiFinal );
        // cv::add( eyeROI, maskedGlass, eyeRoiFinal );

        cv::add( maskedEye, maskedGlass, eyeRoiFinal );

        cv::add( .5 * eyeROI, .5 * maskedGlass, eyeRoiTemp );
        cv::add( .5 * eyeRoiTemp, .75 * eyeRoiFinal, eyeRoiParcial );

        showMat( maskedEye, "Eyes with mask", false );
        showMat( maskedGlass, "Glasses masked", false );
        showMat( eyeRoiFinal, "EyeROI final", false );
        showMat( eyeRoiTemp, "EyeROI temp", false );
        showMat( eyeRoiParcial, "EyeROI par", false );

        // eyeRoiFinal.copyTo( eyeROI );
        eyeRoiParcial.copyTo( eyeROI );

        showMat( faceWithGlassesArithmetic, "EyeROI fina full", true );
    }

    return 0;
}