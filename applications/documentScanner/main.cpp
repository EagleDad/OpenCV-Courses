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

//
// Function declarations
//
void updateView( const cv::Mat& result );
void reset( );
std::vector< cv::Point > detectDocument( );
void processDocument( );

//
// UI Interface
//
std::string windowName = "Document Scanner";
std::string windowNameResult = "Result";
std::vector< std::string > help;
std::vector< std::string > headerTitle;
int textPixelHeight = 20;
int fontFace = cv::FONT_HERSHEY_SIMPLEX;
cv::Scalar fontColor = cv::Scalar( 0, 255, 0 );
int fontThickness = 2;
int targetWidth = 500;
int targetHeight { };

//
// Global images
//
cv::Mat sourceImage;
cv::Mat resultImage;
cv::Mat overlayImage;
cv::Mat combinedImage;
cv::Mat helpImage;
cv::Mat headerImage;

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    help.emplace_back( "Press 'd' for detect and warp document" );
    help.emplace_back( "Press 'r' for reset" );
    help.emplace_back( "Press 'ESC' for exit" );

    headerTitle.emplace_back( "Original Image" );
    headerTitle.emplace_back( "Overlay Image" );
    headerTitle.emplace_back( "Result Image" );

    // The paper we want to scan is in A4 format
    //  Width: 210mm eq 500px
    //  Height: 297mm eq x px

    // 500 px   x px            500px * 297mm
    // ------ : ----- -> x px = -------------- = 707px
    // 210mm    297mm                210mm

    targetHeight = static_cast< int >(
        std::round( static_cast< float >( targetWidth ) * 297.0f / 210.0f ) );

    sourceImage = cv::imread( IMAGES_ROOT + "/scanned-form.jpg" );

    // Resize the input image to the target size to be able to create a nice
    // view application
    cv::resize(
        sourceImage, sourceImage, cv::Size( targetWidth, targetHeight ) );

    resultImage = cv::Mat( sourceImage.size( ), CV_8UC3, cv::Scalar::all( 0 ) );
    overlayImage = sourceImage.clone( );

    cv::namedWindow( windowName );

    updateView( resultImage );

    int key { };
    while ( key != 27 )
    {
        if ( key == 'd' )
        {
            processDocument( );
        }

        if ( key == 'r' )
        {
            reset( );
        }

        key = cv::waitKey( 20 ) & 0xFF;
    }

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}

void updateView( const cv::Mat& result )
{
    cv::hconcat( sourceImage, overlayImage, combinedImage );
    cv::hconcat( combinedImage, result, combinedImage );

    if ( helpImage.size( ).width == 0 || helpImage.size( ).height == 0 )
    {
        const double fontScale = cv::getFontScaleFromHeight(
            fontFace, textPixelHeight, fontThickness );

        int helpImageHeight = textPixelHeight / 2;

        for ( const auto& text : help )
        {
            std::ignore = text;
            helpImageHeight += textPixelHeight;
            helpImageHeight += textPixelHeight / 2;
        }

        helpImageHeight += textPixelHeight / 2;

        helpImage = cv::Mat( helpImageHeight,
                             combinedImage.size( ).width,
                             CV_8UC3,
                             cv::Scalar::all( 0 ) );

        int textPosY = textPixelHeight + textPixelHeight / 2;
        const int textPosX = textPixelHeight;

        for ( auto& text : help )
        {
            putText( helpImage,
                     text,
                     cv::Point( textPosX, textPosY ),
                     fontFace,
                     fontScale,
                     fontColor,
                     fontThickness,
                     cv::LINE_AA );

            textPosY += textPixelHeight + textPixelHeight / 2;
        }
    }

    if ( headerImage.size( ).width == 0 || headerImage.size( ).height == 0 )
    {
        const int headerImageHeight = 2 * textPixelHeight;

        headerImage = cv::Mat( headerImageHeight,
                               combinedImage.size( ).width,
                               CV_8UC3,
                               cv::Scalar::all( 0 ) );

        int xOffset = textPixelHeight;
        const int yPos = textPixelHeight + textPixelHeight / 2;

        const double fontScale = cv::getFontScaleFromHeight(
            fontFace, textPixelHeight, fontThickness );

        for ( auto& text : headerTitle )
        {
            putText( headerImage,
                     text,
                     cv::Point( xOffset, yPos ),
                     fontFace,
                     fontScale,
                     fontColor,
                     fontThickness,
                     cv::LINE_AA );

            xOffset += sourceImage.size( ).width;
        }
    }

    cv::vconcat( headerImage, combinedImage, combinedImage );

    cv::vconcat( combinedImage, helpImage, combinedImage );

    cv::imshow( windowName, combinedImage );
}

void reset( )
{
    resultImage = sourceImage.clone( );
    overlayImage = resultImage.clone( );
    updateView( resultImage );
}

std::vector< cv::Point > detectDocument( )
{
    // Convert the input image to Grayscale
    cv::Mat imgGray;
    cv::cvtColor( sourceImage, imgGray, cv::COLOR_BGR2GRAY );

    cv::Mat imgSegmented;
    cv::threshold(
        imgGray, imgSegmented, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU );

    // Find all contours in the image
    std::vector< std::vector< cv::Point > > contours;
    std::vector< cv::Vec4i > hierarchy;

    cv::findContours( imgSegmented,
                      contours,
                      hierarchy,
                      cv::RETR_EXTERNAL,
                      cv::CHAIN_APPROX_SIMPLE );

    // We are only interested in the contour with the max area
    double maxArea = std::numeric_limits< double >::min( );
    size_t maxIdx { };
    for ( size_t i = 0; i < contours.size( ); i++ )
    {
        const auto area = cv::contourArea( contours[ i ] );

        if ( area > maxArea )
        {
            maxArea = area;
            maxIdx = i;
        }
    }

    cv::drawContours( overlayImage,
                      contours,
                      static_cast< int >( maxIdx ),
                      cv::Scalar( 255, 0, 0 ),
                      2 );

    // To eliminate defects based on segmentation we can make the contour convex
    // at first.
    std::vector< cv::Point > contourConvex;
    cv::convexHull( contours[ maxIdx ], contourConvex );

    // We expect 4 extreme points
    // We can run in a few iterations to make sure we get the 4 extreme points
    int32_t maxIterations = 10;
    double epsilon = 0.5;

    std::vector< cv::Point > contourApprox;
    while ( maxIterations-- > 0 && contourApprox.size( ) != 4 )
    {
        cv::approxPolyDP( contourConvex, contourApprox, epsilon, true );
        epsilon += 0.5;
    }

    // Now we need to get the points in the right order to be able to wrap the
    // image

    // search for the left point
    int32_t posXMin = std::numeric_limits< int32_t >::max( );
    size_t leftMostIdx { };
    for ( size_t i = 0; i < contourApprox.size( ); i++ )
    {
        if ( contourApprox[ i ].x < posXMin )
        {
            posXMin = contourApprox[ i ].x;
            leftMostIdx = i;
        }
    }

    // Check the distances to the two neighbors. One must be bigger that the
    // other
    const auto current = contourApprox[ leftMostIdx ];
    const auto currentP = contourApprox[ ( leftMostIdx + 1 ) % 4 ];
    size_t idxM { };
    if ( leftMostIdx == 0 )
    {
        idxM = 3;
    }
    else
    {
        idxM = leftMostIdx - 1;
    }
    const auto currentM = contourApprox[ idxM ];

    const auto distM = cv::norm( current - currentM );
    const auto distP = cv::norm( current - currentP );

    // Order the points
    std::vector< cv::Point > finalPoints;
    if ( distM < distP )
    {
        finalPoints.emplace_back( contourApprox[ leftMostIdx ] );
        finalPoints.emplace_back( contourApprox[ ( leftMostIdx + 1 ) % 4 ] );
        finalPoints.emplace_back( contourApprox[ ( leftMostIdx + 2 ) % 4 ] );
        finalPoints.emplace_back( contourApprox[ ( leftMostIdx + 3 ) % 4 ] );
    }
    else
    {
        finalPoints.emplace_back( contourApprox[ leftMostIdx ] );

        for ( int32_t i = 1; i <= 3; i++ )
        {
            const auto tmpIdx = static_cast< int32_t >( leftMostIdx );

            if ( tmpIdx - i <= 0 )
            {
                finalPoints.emplace_back(
                    contourApprox[ static_cast< size_t >( 3 - i ) ] );
            }
            else
            {
                finalPoints.emplace_back( contourApprox[ leftMostIdx - i ] );
            }
        }
    }

    // Draw the 4 extreme points
    const double fontScale =
        cv::getFontScaleFromHeight( fontFace, textPixelHeight, fontThickness );
    int32_t idx { };
    for ( const auto& pt : finalPoints )
    {
        cv::circle( overlayImage, pt, 5, cv::Scalar( 0, 255, 0 ), 2 );

        cv::putText( overlayImage,
                     std::to_string( idx++ ),
                     cv::Point( pt.x + 10, pt.y - 10 ),
                     fontFace,
                     fontScale,
                     fontColor,
                     fontThickness );
    }

    updateView( resultImage );

    return finalPoints;
}

void processDocument( )
{
    const auto finalPoints = detectDocument( );

    if ( finalPoints.size( ) != 4 )
    {
        const double fontScale = cv::getFontScaleFromHeight(
            fontFace, textPixelHeight, fontThickness );

        putText( overlayImage,
                 "Invalid number of points detected",
                 cv::Point( 10, 50 ),
                 fontFace,
                 fontScale,
                 cv::Scalar( 0, 0, 255 ),
                 fontThickness,
                 cv::LINE_AA );
    }

    std::vector< cv::Point > destPoints;
    destPoints.emplace_back( 0, 0 );
    destPoints.emplace_back( 0, targetHeight );
    destPoints.emplace_back( targetWidth, targetHeight );
    destPoints.emplace_back( targetWidth, 0 );

    const cv::Mat homography = cv::findHomography( finalPoints, destPoints );

    cv::warpPerspective(
        sourceImage, resultImage, homography, resultImage.size( ) );

    updateView( resultImage );
}