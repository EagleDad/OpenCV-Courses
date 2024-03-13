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

cv::Mat sourceImage;
cv::Mat resultImage;
cv::Mat overlayImage;
cv::Mat combinedImage;
cv::Mat helpImage;
cv::Mat headerImage;
int textPixelHeight = 20;
int fontFace = cv::FONT_HERSHEY_SIMPLEX;
cv::Scalar fontColor = cv::Scalar( 0, 255, 0 );
int fontThickness = 2;
std::string windowName = "Instagram Filters";
bool cartoonifyFilterCv = false;
std::vector< cv::Point2i > patchCenter { };
std::vector< cv::Rect > patchRegions { };
std::vector< std::string > help;
int regionRadius = 15;

std::vector< double >
getAverageGradient( const std::vector< cv::Rect >& regions )
{
    std::vector< double > averages;

    for ( const auto& region : regions )
    {
        // Crop the current region
        cv::Mat roi = sourceImage( region );

        cv::Mat roiGray;
        cv::cvtColor( roi, roiGray, cv::COLOR_BGR2GRAY );

        cv::Mat SobelX;
        cv::Mat SobelY;

        // Step 3:
        cv::Sobel( roiGray, SobelX, CV_32F, 1, 0 );
        cv::Sobel( roiGray, SobelY, CV_32F, 0, 1 );

        // Step 4:
        cv::Mat magnitudeImage;
        cv::Mat angleImage;
        cv::cartToPolar( SobelX, SobelY, magnitudeImage, angleImage );

        cv::Scalar mean;
        cv::Scalar stdDev;

        cv::meanStdDev( magnitudeImage, mean, stdDev );

        averages.push_back( mean[ 0 ] );
    }

    return averages;
}

std::vector< cv::Rect >
getBlemishNeighbourRegions( const cv::Rect& centerRegion )
{
    std::vector< cv::Rect > returnRegions;

    const int xOrg = centerRegion.x;
    const int yOrg = centerRegion.y;
    const int size = 2 * regionRadius;

    for ( int y = -1; y <= 1; y++ )
    {
        for ( int x = -1; x <= 1; x++ )
        {
            if ( x == 0 && y == 0 )
            {
                continue;
            }

            const int xCur = xOrg + size * x;
            const int yCur = yOrg + size * y;

            // Make sure that the regions are in range
            if ( xCur < 0 || yCur < 0 ||
                 xCur + size >= sourceImage.size( ).width ||
                 yCur + size >= sourceImage.size( ).height )
            {
                continue;
            }

            cv::Rect curRect( xCur, yCur, size, size );

            returnRegions.push_back( curRect );

            cv::rectangle( overlayImage,
                           curRect,
                           cv::Scalar( 0, 0, 255 ),
                           1,
                           cv::LINE_AA );
        }
    }

    return returnRegions;
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

        std::vector< std::string > headerTitle;
        headerTitle.push_back( "Original Image" );
        headerTitle.push_back( "Overlay Image showing regions" );
        headerTitle.push_back( "Result Image" );

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

void pencilSketchFilter( )
{
    /*
     * Pencil Sketch Filter
     *
     * The output of the filter should show edges in black and surfaces in
     * white. A normal edge filter will show edges in white and surfaces in
     * black. To achieve the goal, the edge image needs to be inverted.
     *
     * Step 1:
     *      Convert the input image to grayscale, we expect a BGR as input for
     * now.
     *
     * Step 2:
     *     Apply a bilateral filter to smooth the surface and preserver the
     * edges.
     *
     * Step 3:
     *      Calculate Sobel X and Sobel Y to get the edges in X and Y direction.
     *
     * Step 4:
     *      Calculate the gradient magnitude to get one image sowing edges in X
     * and Y.
     *
     * Step 5:
     *      Convert the magnitude image to a grayscale range to be able to
     *      visualize it properly.
     *
     * Step 6:
     *      Invert the image to get black edges and white surfaces.
     *
     * Step 7:
     *      Convert the image back to color to visualize it.
     *
     * NOTE: There might be different solutions achieving the goal. DOG might
     * also work.
     */
    cv::Mat SobelX;
    cv::Mat SobelY;

    // Step 1:
    cv::Mat grayImage;
    cv::cvtColor( sourceImage, grayImage, cv::COLOR_BGR2GRAY );

    // Step 2:
    // diameter of the pixel neighbourhood used during filtering
    const int dia = 5;

    // Larger the value the distant colours will be mixed together
    // to produce areas of semi equal colors
    const double sigmaColor = 20;

    // Larger the value more the influence of the farther placed pixels
    // as long as their colors are close enough
    // If sigma space should be used diameter needs to be -1
    const double sigmaSpace = 20;

    cv::Mat bilateralFiltered;

    // Apply bilateral filter
    cv::bilateralFilter(
        grayImage, bilateralFiltered, dia, sigmaColor, sigmaSpace );

    // Step 3:
    cv::Sobel( bilateralFiltered, SobelX, CV_32F, 1, 0 );
    cv::Sobel( bilateralFiltered, SobelY, CV_32F, 0, 1 );

    // Step 4:
    cv::Mat magnitudeImage;
    cv::Mat angleImage;
    cv::cartToPolar( SobelX, SobelY, magnitudeImage, angleImage );

    // Step 5:
    cv::convertScaleAbs( magnitudeImage, magnitudeImage );

    // Step 6:
    cv::bitwise_not( magnitudeImage, magnitudeImage );

    // Step 7:
    cv::Mat dstMag3C;
    cv::cvtColor( magnitudeImage, dstMag3C, cv::COLOR_GRAY2BGR );

    updateView( dstMag3C );
}

void cartoonifyFilter( )
{
    /*
     * Cartoonify Filter
     *
     * The output of the filter should show an image with enhanced edges a
     * surfaces of constant color. The easiest way would be to use
     * cv::stylization which gives very nice results. Doing it from scratch
     * means we have to apply an edge preserving filter to the image. We use the
     * bilateralFilter with big radius and sigma to get surfaces of constant
     * color. To enhance edges, we apply a bit mask of a segmented image to the
     * final output. We use an adaptiveThreshold to be more robust against
     * non-homogeneous background.
     *
     * Step 1:
     *      Convert image to gray to apply threshold methode.
     *
     * Step 2:
     *      Apply adaptiveThreshold
     *
     * Step 3:
     *      Apply bilateral filter to get surfaces of constant color and
     *      preserve edges.
     *
     * Step 4:
     *      Convert the segmented image to color to apply it as mask.
     *
     * Step 5:
     *      Calculate bitwise and of mask and filtered image to enhance edges.
     */

    if ( cartoonifyFilterCv )
    {
        cv::stylization( sourceImage, resultImage );
        updateView( resultImage );
        return;
    }

    // Step 1:
    cv::Mat grayImage;
    cv::cvtColor( sourceImage, grayImage, cv::COLOR_BGR2GRAY );

    // Step 2:
    cv::Mat segmentedImage;
    cv::adaptiveThreshold( grayImage,
                           segmentedImage,
                           255,
                           cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                           cv::THRESH_BINARY,
                           15,
                           5 );

    // Step 3:
    // diameter of the pixel neighbourhood used during filtering
    const int dia = 15;

    // Larger the value the distant colours will be mixed together
    // to produce areas of semi equal colors
    const double sigmaColor = 80;

    // Larger the value more the influence of the farther placed pixels
    // as long as their colors are close enough
    // If sigma space should be used diameter needs to be -1
    const double sigmaSpace = 80;

    cv::Mat bilateralFiltered;

    // Apply bilateral filter
    cv::bilateralFilter(
        sourceImage, bilateralFiltered, dia, sigmaColor, sigmaSpace );

    // Step 4:
    cv::cvtColor( segmentedImage, segmentedImage, cv::COLOR_GRAY2BGR );

    // Step 5
    cv::bitwise_and( bilateralFiltered, segmentedImage, resultImage );

    updateView( resultImage );
}

void blemishRemovalSeamlessCloning( )
{
    for ( size_t i = 0; i < patchRegions.size( ); i++ )
    {
        const auto neighbourRegions =
            getBlemishNeighbourRegions( patchRegions[ i ] );

        const auto averageGradients = getAverageGradient( neighbourRegions );

        // Get the minimum
        const auto minAvg = std::min_element( averageGradients.begin( ),
                                              averageGradients.end( ) );

        const auto minIdx = static_cast< size_t >(
            std::distance( averageGradients.begin( ), minAvg ) );

        cv::rectangle( overlayImage,
                       neighbourRegions[ minIdx ],
                       cv::Scalar( 0, 255, 0 ),
                       1,
                       cv::LINE_AA );

        cv::Mat srcRoi = resultImage( neighbourRegions[ minIdx ] );
        cv::Mat srcMask =
            cv::Mat( srcRoi.size( ), CV_8UC3, cv::Scalar::all( 255 ) );

        cv::seamlessClone( srcRoi,
                           resultImage,
                           srcMask,
                           patchCenter[ i ],
                           resultImage,
                           cv::NORMAL_CLONE );
    }

    updateView( resultImage );
}

void blemishRemovalInPainting( )
{
    cv::Mat inpaintMask = cv::Mat::zeros( sourceImage.size( ), CV_8U );

    for ( size_t i = 0; i < patchCenter.size( ); i++ )
    {
        cv::circle(
            inpaintMask, patchCenter[ i ], 15, cv::Scalar::all( 255 ), -1 );
    }

    cv::inpaint( sourceImage, inpaintMask, resultImage, 3, cv::INPAINT_NS );

    updateView( resultImage );
}

void reset( )
{
    overlayImage = sourceImage.clone( );
    resultImage = sourceImage.clone( );
    patchCenter.clear( );
    patchRegions.clear( );
    updateView( sourceImage );
}

void onMouse( int action, int x, int y, [[maybe_unused]] int flags,
              [[maybe_unused]] void* userdata )
{
    if ( action == cv::EVENT_LBUTTONDOWN )
    {
        const int yPosOffset = headerImage.size( ).height;

        if ( y > sourceImage.size( ).height + yPosOffset || y < yPosOffset ||
             x > sourceImage.size( ).width )
        {
            std::cout << "Marker should be placed in original image.\n";
            return;
        }

        y -= headerImage.size( ).height;

        const cv::Point2i center( x, y );
        patchCenter.push_back( center );

        const cv::Rect region( x - regionRadius,
                               y - regionRadius,
                               2 * regionRadius,
                               2 * regionRadius );

        patchRegions.push_back( region );

        cv::circle(
            overlayImage, center, 1, cv::Scalar( 255, 0, 0 ), 5, cv::LINE_AA );

        cv::rectangle(
            overlayImage, region, cv::Scalar( 255, 0, 0 ), 5, cv::LINE_AA );

        updateView( resultImage );
    }
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    help.push_back( "Press 'p' for pencil sketch" );
    help.push_back( "Press 'c' for cartoon sketch" );
    help.push_back( "Press 'i' for blemish removal using in-painting." );
    help.push_back( "Press 's' for blemish removal using seamless cloning." );
    help.push_back( "Press 'r' for reset" );
    help.push_back( "Press 'ESC' for exit" );
    help.push_back( "" );
    help.push_back( "Note: For blemish removal use left mouse button in "
                    "original image to place regions to be corrected." );

    int key { };
    cv::namedWindow( windowName );
    sourceImage = cv::imread( IMAGES_ROOT + "/blemish.png" );
    resultImage = sourceImage.clone( );
    overlayImage = sourceImage.clone( );
    updateView( resultImage );

    // highgui function called when mouse events occur
    cv::setMouseCallback( windowName, onMouse );

    while ( key != 27 )
    {
        if ( key == 'p' )
        {
            pencilSketchFilter( );
        }

        if ( key == 'c' )
        {
            cartoonifyFilter( );
        }

        if ( key == 'i' )
        {
            blemishRemovalInPainting( );
        }

        if ( key == 's' )
        {
            resultImage = sourceImage.clone( );
            blemishRemovalSeamlessCloning( );
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