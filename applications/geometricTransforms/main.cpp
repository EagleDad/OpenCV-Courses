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
    // Create a black image of size 200x200
    cv::Mat img( 200, 200, CV_32FC3, cv::Scalar( 0, 0, 0 ) );

    // Create a blue square in the center
    img( cv::Range( 50, 150 ), cv::Range( 50, 150 ) )
        .setTo( cv::Scalar( 1, 0.6, 0.2 ) );

    // Display image
    showMat( img, "Rectangle", true );

    // Translate the image
    // Output dimension
    cv::Size outDim = img.size( );

    // Translate by 25,25
    float warpMatValues[] = { 1.0, 0.0, 25.0, 0.0, 1.0, 25.0 };
    cv::Mat warpMat = cv::Mat( 2, 3, CV_32F, warpMatValues );

    // Warp Image
    cv::Mat result;
    cv::warpAffine( img, result, warpMat, outDim );

    // Display image
    showMat( result, "Translated Rectangle", true );

    // Scale the image
    // Scale along x direction
    float warpMatValues2[] = { 2.0, 0.0, 0.0, 0.0, 1.0, 0.0 };
    warpMat = cv::Mat( 2, 3, CV_32F, warpMatValues2 );

    // Warp image
    cv::warpAffine( img, result, warpMat, outDim );

    // Display image
    showMat( result, "Scaled Rectangle", true );

    // Scale image width
    // Scale along x direction

    // Warp image
    cv::warpAffine(
        img, result, warpMat, cv::Size( outDim.width * 2, outDim.height ) );

    // Display image
    showMat( result, "Scaled Rectangle and size", true );

    // Scale along both dimensions
    // Scale along x and y directions
    float warpMatValues3[] = { 2.0, 0.0, 0.0, 0.0, 2.0, 0.0 };
    warpMat = cv::Mat( 2, 3, CV_32F, warpMatValues3 );

    // Warp image
    cv::warpAffine(
        img, result, warpMat, cv::Size( outDim.width * 2, outDim.height * 2 ) );

    // Display image
    showMat( result, "Scaled Rectangle x and y", true );

    // Rotate Image about the origin (0,0)
    // Rotate image
    float angleInRadians = 30.f;
    angleInRadians = 30.f * 3.14f / 180.0f;

    float cosTheta = cos( angleInRadians );
    float sinTheta = sin( angleInRadians );

    // Rotation matrix
    // https://en.wikipedia.org/wiki/Rotation_matrix

    float warpMatValues4[] = {
        cosTheta, sinTheta, 0.0, -sinTheta, cosTheta, 0.0 };
    warpMat = cv::Mat( 2, 3, CV_32F, warpMatValues4 );

    // Warp image
    cv::warpAffine( img, result, warpMat, outDim );

    // Display image
    showMat( result, "Rotated Rectangle origin", true );

    // Rotate image about a specific point (center)
    float centerX = static_cast< float >( img.size( ).width ) / 2.0f;
    float centerY = static_cast< float >( img.size( ).height ) / 2.0f;

    float tx = ( 1 - cosTheta ) * centerX - sinTheta * centerY;
    float ty = sinTheta * centerX + ( 1 - cosTheta ) * centerY;

    // Rotation matrix
    // https://en.wikipedia.org/wiki/Rotation_matrix

    float warpMatValues5[] = {
        cosTheta, sinTheta, tx, -sinTheta, cosTheta, ty };
    warpMat = cv::Mat( 2, 3, CV_32F, warpMatValues5 );

    // Warp image
    cv::warpAffine( img, result, warpMat, outDim );

    // Display image
    showMat( result, "Rotated Rectangle center", true );

    // Rotate image the easy way
    float anglesInDegrees = 30;
    // Get rotation matrix
    cv::Mat rotationMatrix = getRotationMatrix2D(
        cv::Point2f( centerX, centerY ), anglesInDegrees, 1 );

    // Warp image
    cv::warpAffine( img, result, rotationMatrix, outDim );

    showMat( result, "Rotated Rectangle center cv", true );

    // Shear Transformation
    float shearAmount = 0.1f;

    float warpMatValues6[] = { 1, shearAmount, 0, 0, 1.0, 0 };
    warpMat = cv::Mat( 2, 3, CV_32F, warpMatValues6 );

    // Warp image
    cv::warpAffine( img, result, warpMat, outDim );

    showMat( result, "Sheared Rectangle", true );

    // Complex Transformations
    // Scale
    float scaleAmount = 1.1f;
    float warpMatValues7[] = { scaleAmount, 0.0, 0, scaleAmount };
    cv::Mat scaleMat = cv::Mat( 2, 2, CV_32F, warpMatValues7 );

    // Shear
    shearAmount = -0.1f;
    float warpMatValues8[] = { 1, shearAmount, 0, 1.0 };
    cv::Mat shearMat = cv::Mat( 2, 2, CV_32F, warpMatValues8 );

    // Rotate by 10 degrees about (0,0)
    angleInRadians = 10.0f * 3.14f / 180.0f;
    cosTheta = cos( angleInRadians );
    sinTheta = sin( angleInRadians );

    float warpMatValues9[] = { cosTheta, sinTheta, -sinTheta, cosTheta };
    cv::Mat rotMat = cv::Mat( 2, 2, CV_32F, warpMatValues9 );

    float warpMatValues10[] = { 10, 0 };
    cv::Mat translateVector = cv::Mat( 2, 1, CV_32F, warpMatValues10 );

    // First scale is applied, followed by shear, followed by rotation.
    cv::Mat scaleShearRotate = rotMat * shearMat * scaleMat;

    cv::hconcat( scaleShearRotate, translateVector, warpMat );
    std::cout << warpMat << "\n";

    float warpMatValues11[] = { 50, 50, 50, 149, 149, 50, 149, 149 };

    cv::hconcat( translateVector, translateVector, translateVector );
    cv::hconcat( translateVector, translateVector, translateVector );
    cv::Mat outPts =
        scaleShearRotate * cv::Mat( 4, 2, CV_32F, warpMatValues11 ).t( ) +
        translateVector;
    std::cout << outPts << "\n";

    // Warp image
    cv::warpAffine( img, result, warpMat, outDim );

    showMat( result, "Complex transformed Rectangle", true );

    // Complex Transformations using 3-Point Correspondences
    // 6 points = 6 degrees of freedom. Point has x and y

    /*
     * We know that an affine transform that 6 degrees of freedom
     *
     *   1. Two for translation (tx, ty)
     *   2. Two for scale (sx, sy)
     *   3. One for shear
     *   4. One for in-plane rotation
     */
    std::vector< cv::Point2f > srcPoints {
        cv::Point2f( 50, 50 ), cv::Point2f( 50, 149 ), cv::Point2f( 149, 50 ) };
    std::vector< cv::Point2f > dstPoints {
        cv::Point2f( 68, 45 ), cv::Point2f( 76, 155 ), cv::Point2f( 176, 27 ) };

    cv::Mat estimatedMat = cv::estimateAffine2D( srcPoints, dstPoints );

    std::cout << "True warp matrix:\n\n" << warpMat << "\n";
    std::cout << "Estimated warp matrix:\n\n" << estimatedMat << "\n";

    srcPoints = std::vector< cv::Point2f > {cv::Point2f( 50, 50 ),
                                            cv::Point2f( 50, 149 ),
                                            cv::Point2f( 149, 149 ),
                                            cv::Point2f( 149, 50 ) };
    dstPoints = std::vector< cv::Point2f > {cv::Point2f( 68, 45 ),
                                            cv::Point2f( 76, 155 ),
                                            cv::Point2f( 183, 135 ),
                                            cv::Point2f( 176, 27 ) };

    estimatedMat = cv::estimateAffine2D( srcPoints, dstPoints );

    std::cout << "True warp matrix:\n\n" << warpMat << "\n";

    std::cout << "Estimated warp matrix:\n\n" << estimatedMat << "\n";

    // Warp image
    cv::warpAffine( img, result, estimatedMat, outDim );

    showMat( result, "Complex transformed Rectangle E", true );

    // Limitations of Affine Transform
    // Transformed image
    cv::Mat imgT( 200, 200, CV_32FC3, cv::Scalar( 0, 0, 0 ) );

    cv::Point dstPoints2[] = {cv::Point( 75, 50 ),
                              cv::Point( 50, 149 ),
                              cv::Point( 149, 149 ),
                              cv::Point( 124, 50 ) };

    cv::fillConvexPoly( imgT, dstPoints2, 4, cv::Scalar( 1.0, 0.6, 0.2 ), cv::LINE_AA );

    showMat( img, "Original Image", false );
    showMat( imgT, "Transformed Image", false );

    dstPoints = std::vector< cv::Point2f > {cv::Point2f( 75, 50 ),
                                            cv::Point2f( 50, 149 ),
                                            cv::Point2f( 149, 149 ),
                                            cv::Point2f( 124, 50 ) };
    estimatedMat = cv::estimateAffine2D( srcPoints, dstPoints );

    std::cout << "Estimated warp matrix:\n\n" << estimatedMat << "\n";

    // Warp image
    cv::Mat imA;
    cv::warpAffine( img, imA, estimatedMat, outDim );
    showMat( imA, "Image warped using estimated Affine Transform", false );

    cv::Mat h = findHomography( srcPoints, dstPoints );
    std::cout << h << "\n";

    // Warp source image to destination based on homography
    cv::Mat imH;
    cv::warpPerspective( img, imH, h, outDim );
    showMat( imH, "Image warped using estimated Homography", true );

    // Homography Example
    // Read source image.
    cv::Mat im_src = cv::imread( IMAGES_ROOT + "/book2.jpg" );
    // Four corners of the book in source image
    std::vector< cv::Point2f > pts_src {cv::Point2f( 141, 131 ),
                                        cv::Point2f( 480, 159 ),
                                        cv::Point2f( 493, 630 ),
                                        cv::Point2f( 64, 601 ) };

    // Read destination image.
    cv::Mat im_dst = cv::imread( IMAGES_ROOT + "/book1.jpg" );
    // Four corners of the book in destination image.
    std::vector< cv::Point2f > pts_dst {cv::Point2f( 318, 256 ),
                                        cv::Point2f( 534, 372 ),
                                        cv::Point2f( 316, 670 ),
                                        cv::Point2f( 73, 473 ) };

    // Calculate Homography
    h = findHomography( pts_src, pts_dst );

    // Warp source image to destination based on homography
    cv::Mat im_out;
    cv::warpPerspective( im_src, im_out, h, im_dst.size( ) );

    showMat( im_src, "Source Image", false );
    showMat( im_dst, "Destination Image", false );
    showMat( im_out, "Warped Image", true );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}