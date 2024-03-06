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
    std::string imagePath = IMAGES_ROOT + "/Contour.png";

    cv::Mat image = cv::imread( imagePath );
    cv::Mat imageCopy = image.clone( );

    cv::Mat imageGray;
    // Convert to grayscale
    cv::cvtColor( image, imageGray, cv::COLOR_BGR2GRAY );

    showMat( image, "Original Image" );

    showMat( imageGray, "Grayscale Image" );

    // Find all contours in the image
    std::vector< std::vector< cv::Point > > contours;
    std::vector< cv::Vec4i > hierarchy;

    cv::findContours( imageGray,
                      contours,
                      hierarchy,
                      cv::RETR_LIST,
                      cv::CHAIN_APPROX_SIMPLE );

    std::cout << "Number of contours found = " << contours.size( ) << "\n";

    cv::drawContours( image, contours, -1, cv::Scalar( 0, 255, 0 ), 6 );

    showMat( image, "Image with contours RETR_LIST" );

    // Find external contours in the image
    cv::findContours( imageGray,
                      contours,
                      hierarchy,
                      cv::RETR_EXTERNAL,
                      cv::CHAIN_APPROX_SIMPLE );

    std::cout << "Number of contours found = " << contours.size( ) << "\n";

    image = imageCopy.clone( );

    // Draw all the contours
    cv::drawContours( image, contours, -1, cv::Scalar( 0, 255, 0 ), 3 );

    showMat( image, "Image with contours RETR_EXTERNAL" );

    // Draw only the 3rd contour
    // Note that right now we do not know
    // the numbering of contour in terms of the shapes
    // present in the figure
    image = imageCopy.clone( );
    cv::drawContours( image, contours, 2, cv::Scalar( 0, 0, 255 ), 3 );

    showMat( image, "Image with contours[2] RETR_EXTERNAL" );

    // Find all contours in the image
    cv::findContours( imageGray,
                      contours,
                      hierarchy,
                      cv::RETR_LIST,
                      cv::CHAIN_APPROX_SIMPLE );

    // Draw all the contours
    drawContours( image, contours, -1, cv::Scalar( 0, 255, 0 ), 3 );

    cv::Moments M;
    int x, y;
    for ( size_t i = 0; i < contours.size( ); i++ )
    {
        // We will use the contour moments
        // to find the centroid
        M = moments( contours[ i ] );
        x = static_cast< int >( M.m10 / M.m00 );
        y = static_cast< int >( M.m01 / M.m00 );

        // Mark the center
        cv::circle( image, cv::Point( x, y ), 10, cv::Scalar( 255, 0, 0 ), -1 );
    }

    showMat( image, "Image with contours RETR_LIST" );

    for ( size_t i = 0; i < contours.size( ); i++ )
    {
        // We will use the contour moments
        // to find the centroid
        M = moments( contours[ i ] );
        x = static_cast< int >( M.m10 / M.m00 );
        y = static_cast< int >( M.m01 / M.m00 );

        // Mark the center
        cv::circle( image, cv::Point( x, y ), 10, cv::Scalar( 255, 0, 0 ), -1 );

        // Mark the contour number
        cv::putText( image,
                     std::to_string( i + 1 ),
                     cv::Point( x + 40, y - 10 ),
                     cv::FONT_HERSHEY_SIMPLEX,
                     1,
                     cv::Scalar( 0, 0, 255 ),
                     2 );
    }

    showMat( image, "Image with contours RETR_LIST Lbl" );

    imageCopy = image.clone( );

    // Area and Perimeter
    double area;
    double perimeter;
    for ( size_t i = 0; i < contours.size( ); i++ )
    {
        area = cv::contourArea( contours[ i ] );
        perimeter = cv::arcLength( contours[ i ], true );
        std::cout << "Contour #" << i + 1 << " has area = " << area
                  << " and perimeter = " << perimeter << '\n';
    }

    // Bounding Boxes
    image = imageCopy.clone( );
    cv::Rect rect;
    for ( size_t i = 0; i < contours.size( ); i++ )
    {
        // Vertical rectangle
        rect = boundingRect( contours[ i ] );
        cv::rectangle( image, rect, cv::Scalar( 255, 0, 255 ), 2 );
    }

    showMat( image, "Image with contours bounding boxes" );

    image = imageCopy.clone( );
    cv::RotatedRect rotrect;
    cv::Point2f rect_points[ 4 ];
    cv::Mat boxPoints2f, boxPointsCov;

    for ( size_t i = 0; i < contours.size( ); i++ )
    {
        // Rotated rectangle
        rotrect = cv::minAreaRect( contours[ i ] );
        cv::boxPoints( rotrect, boxPoints2f );
        boxPoints2f.assignTo( boxPointsCov, CV_32S );
        cv::polylines(
            image, boxPointsCov, true, cv::Scalar( 0, 255, 255 ), 2 );
    }

    showMat( image, "Image with contours min area rects" );

    // Fit a circle and ellipse
    image = imageCopy.clone( );
    cv::Point2f center;
    float radius;
    for ( size_t i = 0; i < contours.size( ); i++ )
    {
        // Fit a circle
        cv::minEnclosingCircle( contours[ i ], center, radius );
        cv::circle( image,
                    center,
                    static_cast< int >( radius ),
                    cv::Scalar( 125, 125, 125 ),
                    2 );
    }

    showMat( image, "Image with contours enclosing circle" );

    image = imageCopy.clone( );
    cv::RotatedRect rellipse;
    for ( size_t i = 0; i < contours.size( ); i++ )
    {
        // Fit an ellipse
        // We can fit an ellipse only
        // when our contour has minimum
        // 5 points
        if ( contours[ i ].size( ) < 5 )
            continue;
        rellipse = cv::fitEllipse( contours[ i ] );
        cv::ellipse( image, rellipse, cv::Scalar( 255, 0, 125 ), 2 );
    }

    showMat( image, "Image with contours fitted ellipse", true );

    // Clean up
    //system( "pause" ); // NOLINT(concurrency-mt-unsafe)
    cv::destroyAllWindows( );

    return 0;
}