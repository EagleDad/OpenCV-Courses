#include <GUI.h>
#include <macros.h>

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
IGNORE_WARNINGS_POP

// STD includes
#include <iostream>

const std::string IMAGES_ROOT = "C:/images";
const std::string RESULTS_ROOT = "C:/images/results";

// Run variable that will stop the app after storing the ROI
bool run = true;
cv::Mat source;

void resizeImage( [[maybe_unused]] int action, [[maybe_unused]] int x,
                  [[maybe_unused]] int y, [[maybe_unused]] int flags,
                  [[maybe_unused]] void* userdata )
{
    // Action to be taken when left mouse button is pressed
    if ( ( action == cv::EVENT_MOUSEWHEEL ) &&
         ( flags & cv::EVENT_FLAG_CTRLKEY ) )
    {
        std::cout << "Mouse and left button down\n";

        // Action to be taken when ctrl key + mouse wheel scrolled forward
        if ( cv::getMouseWheelDelta( flags ) > 0 )
        {
            std::cout << "Zoom in\n";
        }
        // Resize image
        // Action to be taken when ctrl key + mouse wheel scrolled backward
        else
        {
            std::cout << "Zoom out\n";
        }
        // Resize image
    }
}

void addImages( )
{
    // Matrix as an Image
    const std::string imagePath = IMAGES_ROOT + "/tile.png";

    // Read image in Grayscale format
    cv::Mat image = cv::imread( imagePath, 1 );

    showMat( image, "Image In", false, 1 );

    // Convert to RGB
    cv::Mat imageRGB;
    cv::cvtColor( image, imageRGB, cv::COLOR_BGR2RGB );
    // Add 200 to image
    cv::add( imageRGB, 200, imageRGB );

    cv::cvtColor( imageRGB, image, cv::COLOR_RGB2BGR );

    showMat( imageRGB, "Image RGB", true, 1 );
}

void zoomImage( )
{
    // Matrix as an Image
    const std::string imagePath = IMAGES_ROOT + "/tile.png";

    // Read image in Grayscale format
    cv::Mat image = cv::imread( imagePath, 1 );

    cv::namedWindow( "Window" );

    // highgui function called when mouse events occur
    cv::setMouseCallback( "Window", resizeImage );

    source = image.clone( );
    int k = 0;
    // loop until escape character is pressed
    while ( k != 27 && run )
    {
        cv::imshow( "Window", source );

        k = cv::waitKey( 20 ) & 0xFF;

        /*if (  k != 255 )
            std::cout << "Key: " << k << "\n";*/
    }
}

void detectBlobs( )
{
    // Matrix as an Image
    const std::string imagePath = IMAGES_ROOT + "/QuizBlob2.png";

    // Read image in Grayscale format
    cv::Mat image = cv::imread( imagePath, cv::IMREAD_GRAYSCALE );

    showMat( image, "Image In", false, 1 );

    cv::SimpleBlobDetector::Params params;
    params.filterByArea = false;

    const cv::Ptr< cv::SimpleBlobDetector > detector =
        cv::SimpleBlobDetector::create( params );

    std::vector< cv::KeyPoint > keypoints;
    detector->detect( image, keypoints );

    // Mark blobs using image annotation concepts we have studied so far
    int x, y;
    int radius;
    double diameter;
    cv::Mat img = image.clone( );

    cv::cvtColor( image, img, cv::COLOR_GRAY2BGR );

    for ( int i = 0; i < static_cast< int >( keypoints.size( ) ); i++ )
    {
        cv::KeyPoint k = keypoints[ static_cast< size_t >( i ) ];
        cv::Point keyPt;
        keyPt = k.pt;
        x = static_cast< int >( keyPt.x );
        y = static_cast< int >( keyPt.y );
        // Mark center in BLACK
        circle( img, cv::Point( x, y ), 5, cv::Scalar( 255, 0, 0 ), -1 );
        // Get radius of coin
        diameter = k.size;
        radius = static_cast< int >( diameter / 2.0 );
        // Mark blob in GREEN
        cv::circle(
            img, cv::Point( x, y ), radius, cv::Scalar( 0, 255, 0 ), 2 );
    }

    showMat( img, "Blobs", true );
}

void carWithMesh( )
{
    // Matrix as an Image
    const std::string imagePath = IMAGES_ROOT + "/carWithMesh.png";

    // Read image in Grayscale format
    cv::Mat image = cv::imread( imagePath, cv::IMREAD_COLOR );

    showMat( image, "Image In", false, 1 );

    cv::Mat channels[ 3 ];
    cv::split( image, channels );

    cv::Mat imageB = channels[ 0 ];
    cv::Mat imageG = channels[ 1 ];
    cv::Mat imageR = channels[ 2 ];

    showMat( imageB, "Image Blue", false, 1 );
    showMat( imageG, "Image Green", false, 1 );
    showMat( imageR, "Image Red", true, 1 );

    cv::Mat segmented;
    cv::threshold( imageB, segmented, 10, 255, cv::THRESH_BINARY_INV );
    showMat( segmented, "Segmented", true, 1 );

    std::vector< std::vector< cv::Point > > contours;
    std::vector< cv::Vec4i > hierarchy;
    cv::findContours( segmented,
                      contours,
                      hierarchy,
                      cv::RETR_EXTERNAL,
                      cv::CHAIN_APPROX_SIMPLE );

    // Draw all the contours
    cv::Mat imageWithContours = image.clone( );

    cv::drawContours(
        imageWithContours, contours, -1, cv::Scalar( 0, 255, 0 ), 3 );

    showMat( imageWithContours, "Image with contours", true );

    double areaMax = std::numeric_limits< double >::min( );
    size_t idxMax { };
    for ( size_t i = 0; i < contours.size( ); i++ )
    {
        auto area = cv::contourArea( contours[ i ] );

        if ( area > areaMax )
        {
            areaMax = area;
            idxMax = i;
        }
    }

    imageWithContours = image.clone( );

    cv::drawContours( imageWithContours,
                      contours,
                      static_cast< int >( idxMax ),
                      cv::Scalar( 0, 0, 255 ),
                      5 );

    showMat( imageWithContours, "Image with final contours", true );
}

void carWithMesh2( )
{
    // Matrix as an Image
    const std::string imagePath = IMAGES_ROOT + "/carWithMesh.png";

    // Read image in Grayscale format
    cv::Mat image = cv::imread( imagePath, cv::IMREAD_COLOR );

    showMat( image, "Image In", false, 1 );

    cv::Mat channels[ 3 ];
    cv::split( image, channels );

    cv::Mat imageB = channels[ 0 ];
    cv::Mat imageG = channels[ 1 ];
    cv::Mat imageR = channels[ 2 ];

    cv::Mat subtracted;
    cv::subtract( imageB, imageR, subtracted );

    showMat( subtracted, "Image Sub", true, 1 );

    //showMat( imageB, "Image Blue", false, 1 );
    //showMat( imageG, "Image Green", false, 1 );
    //showMat( imageR, "Image Red", true, 1 );

    cv::Mat segmented;
    cv::threshold( subtracted, segmented, 128, 255, cv::THRESH_BINARY );
    showMat( segmented, "Segmented", true, 1 );

    //std::vector< std::vector< cv::Point > > contours;
    //std::vector< cv::Vec4i > hierarchy;
    //cv::findContours( segmented,
    //                  contours,
    //                  hierarchy,
    //                  cv::RETR_EXTERNAL,
    //                  cv::CHAIN_APPROX_SIMPLE );

    //// Draw all the contours
    //cv::Mat imageWithContours = image.clone( );

    //cv::drawContours(
    //    imageWithContours, contours, -1, cv::Scalar( 0, 255, 0 ), 3 );

    //showMat( imageWithContours, "Image with contours", true );

    //double areaMax = std::numeric_limits< double >::min( );
    //size_t idxMax { };
    //for ( size_t i = 0; i < contours.size( ); i++ )
    //{
    //    auto area = cv::contourArea( contours[ i ] );

    //    if ( area > areaMax )
    //    {
    //        areaMax = area;
    //        idxMax = i;
    //    }
    //}

    //imageWithContours = image.clone( );

    //cv::drawContours( imageWithContours,
    //                  contours,
    //                  static_cast< int >( idxMax ),
    //                  cv::Scalar( 0, 0, 255 ),
    //                  5 );

    //showMat( imageWithContours, "Image with final contours", true );
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    carWithMesh2( );

    cv::waitKey( 0 );
    cv::destroyAllWindows( );

    return 0;
}