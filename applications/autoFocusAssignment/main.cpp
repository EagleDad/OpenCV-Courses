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

// Implement Variance of absolute values of Laplacian - Method 1
// Input: image
// Output: Floating point number denoting the measure of sharpness of image

// Do NOT change the function name and definition
double var_abs_laplacian( cv::Mat image )
{
    ///
    /// YOUR CODE HERE
    ///
    // Convert image to grayscale first
    cv::Mat imageGray;
    cv::cvtColor( image, imageGray, cv::COLOR_BGR2GRAY );

    // Calculate laplacian as described in (10)
    cv::Mat lap;
    cv::Laplacian( imageGray, lap, CV_32F, 3 );

    // Get the absolute laplacian image as described in (11)
    const cv::Mat labAbs = cv::abs( lap );

    // Get the mean from the absolute values of the laplacian as described in (13)
    cv::Scalar mean;
    cv::Scalar stdDev;
    cv::meanStdDev( labAbs, mean, stdDev );

    // Calculate standard deviation to get variance a described in (12)
    const cv::Mat sub = cv::abs( labAbs - mean[ 0 ] );
    cv::meanStdDev( sub, mean, stdDev );

    // The standard deviation is the square root of the variance, so we square
    // the value Might be sufficient ti just return stdDev. This is just to
    // follow the formula.
    return stdDev[ 0 ] * stdDev[ 0 ];
}

// Implement Sum Modified Laplacian - Method 2
// Input: image
// Output: Floating point number denoting the measure of sharpness of image

// Do NOT change the function name and definition

double sum_modified_laplacian( cv::Mat image )
{
    ///
    /// YOUR CODE HERE
    ///
    // Convert the image to gray first
    cv::Mat imageGray;
    cv::cvtColor( image, imageGray, cv::COLOR_BGR2GRAY );

    // Define the kernels to be used in (9) and (10)
    cv::Mat kernelX = cv::Mat::zeros( 1, 3, CV_32F );
    cv::Mat kernelY = cv::Mat::zeros( 3, 1, CV_32F );

    kernelX.at< float >( 0, 0 ) = -1.0f;
    kernelX.at< float >( 0, 1 ) = 2.0f;
    kernelX.at< float >( 0, 2 ) = -1.0f;

    kernelY.at< float >( 0, 0 ) = -1.0f;
    kernelY.at< float >( 1, 0 ) = 2.0f;
    kernelY.at< float >( 2, 0 ) = -1.0f;

    // Apply laplacian kernel to input image for x and y separately
    cv::Mat laplacianX;
    cv::filter2D( imageGray, laplacianX, -1, kernelX );

    cv::Mat laplacianY;
    cv::filter2D( imageGray, laplacianY, -1, kernelY );

    // Get the sum of both filters as described on (9) and (10)
    cv::Mat labAbsSum = cv::abs( laplacianX ) + cv::abs( laplacianY );

    // Calculate the sum as described in (11)
    return cv::sum( labAbsSum )[ 0 ];
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    // Read input video filename
    std::string filename = IMAGES_ROOT + "/focus-test.mp4";

    // Create a VideoCapture object
    cv::VideoCapture cap( filename );

    // Read first frame from the video
    cv::Mat frame;
    cap >> frame;

    // Display total number of frames in the video
    std::cout << "Total number of frames : "
              << ( int )cap.get( cv::CAP_PROP_FRAME_COUNT );

    double maxV1 = 0;
    double maxV2 = 0;

    // Frame with maximum measure of focus
    // Obtained using methods 1 and 2
    cv::Mat bestFrame1;
    cv::Mat bestFrame2;

    // Frame ID of frame with maximum measure
    // of focus
    // Obtained using methods 1 and 2
    int bestFrameId1 = 0;
    int bestFrameId2 = 0;

    // Get measures of focus from both methods
    double val1 = var_abs_laplacian( frame );
    double val2 = sum_modified_laplacian( frame );

    // Specify the ROI for flower in the frame
    // UPDATE THE VALUES BELOW
    int topCorner = 0;
    int leftCorner = 0;
    int bottomCorner = frame.size( ).height;
    int rightCorner = frame.size( ).width;

    cv::Mat flower;
    flower = frame( cv::Range( topCorner, bottomCorner ),
                    cv::Range( leftCorner, rightCorner ) );

    // Iterate over all the frames present in the video
    while ( 1 )
    {
        showMat( frame, "Frame", false, 1, 25 );

        // Crop the flower region out of the frame
        flower = frame( cv::Range( topCorner, bottomCorner ),
                        cv::Range( leftCorner, rightCorner ) );
        // Get measures of focus from both methods
        val1 = var_abs_laplacian( flower );
        val2 = sum_modified_laplacian( flower );
        // If the current measure of focus is greater
        // than the current maximum
        if ( val1 > maxV1 )
        {
            // Revise the current maximum
            maxV1 = val1;
            // Get frame ID of the new best frame
            bestFrameId1 = ( int )cap.get( cv::CAP_PROP_POS_FRAMES );
            // Revise the new best frame
            bestFrame1 = frame.clone( );
            std::cout << "Frame ID of the best frame [Method 1]: "
                      << bestFrameId1 << '\n';
        }
        // If the current measure of focus is greater
        // than the current maximum
        if ( val2 > maxV2 )
        {
            // Revise the current maximum
            maxV2 = val2;
            // Get frame ID of the new best frame
            bestFrameId2 = ( int )cap.get( cv::CAP_PROP_POS_FRAMES );
            // Revise the new best frame
            bestFrame2 = frame.clone( );
            std::cout << "Frame ID of the best frame [Method 2]: "
                      << bestFrameId2 << '\n';
        }

        cap >> frame;

        if ( frame.empty( ) )
            break;
    }

    std::cout << "================================================" << '\n';

    // Print the Frame ID of the best frame
    std::cout << "Frame ID of the best frame [Method 1]: " << bestFrameId1
              << '\n';
    std::cout << "Frame ID of the best frame [Method 2]: " << bestFrameId2
              << '\n';

    cap.release( );

    cv::Mat out;
    cv::hconcat( bestFrame1, bestFrame2, out );

    showMat( out, "Frame", true );

    cv::destroyAllWindows( );

    return 0;
}