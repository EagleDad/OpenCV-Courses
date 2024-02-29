#include "macros.h"

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
IGNORE_WARNINGS_POP

// STD includes
#include <iostream>

std::string type2str( int type )
{
    std::string r;

    const auto depth = type & CV_MAT_DEPTH_MASK;
    const auto chans = static_cast< char >( 1 + ( type >> CV_CN_SHIFT ) );

    switch ( depth )
    {
    case CV_8U:
        r = "8U";
        break;
    case CV_8S:
        r = "8S";
        break;
    case CV_16U:
        r = "16U";
        break;
    case CV_16S:
        r = "16S";
        break;
    case CV_32S:
        r = "32S";
        break;
    case CV_32F:
        r = "32F";
        break;
    case CV_64F:
        r = "64F";
        break;
    default:
        r = "User";
        break;
    }

    r += "C";
    r += ( chans + '0' ); // NOLINT(bugprone-narrowing-conversions)

    return r;
}

void showMat( const cv::Mat& imageIn, const std::string& szName,
              bool bWait = false, double scale = 1 )
{
    cv::namedWindow( szName, cv::WINDOW_NORMAL );

    cv::imshow( szName, imageIn );

    cv::resizeWindow( szName,
                      static_cast< int >( imageIn.cols * scale ),
                      static_cast< int >( imageIn.rows * scale ) );

    cv::waitKey( bWait ? 0 : 100 );
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    // Matrix as an Image
    const std::string imagePath = "../../../images/number_zero.jpg";

    // Read image in Grayscale format
    cv::Mat testImage = cv::imread( imagePath, 0 );

    std::cout << testImage << '\n';

    std::cout << "Data type = " << type2str( testImage.type( ) ) << '\n';
    std::cout << "Image Dimensions = " << testImage.size( ) << '\n';

    // Manipulating pixels
    std::cout << static_cast< int32_t >( testImage.at< uchar >( 0, 0 ) )
              << '\n';

    testImage.at< uchar >( 0, 0 ) = 200;

    std::cout << testImage << '\n';

    // Manipulating a group of pixels ROI
    const cv::Mat test_roi = testImage( cv::Range( 0, 2 ), cv::Range( 0, 4 ) );
    std::cout << "Original Matrix\n" << testImage << '\n';

    std::cout << "Selected Region\n" << test_roi << '\n';

    testImage( cv::Range( 0, 2 ), cv::Range( 0, 4 ) ).setTo( 111 );

    std::cout << "Modified Matrix\n" << testImage << '\n';

    // Displaying an image
    showMat( testImage, "Test Image", true, 5 );

    cv::destroyAllWindows( );

    // Additional display utilities
    // See showMat function

    // Saving an image
    cv::imwrite( "c:/tmp/test.jpg", testImage );

    // Color images
    const std::string colImagePath = "../../../images/musk.jpg";

    // Read the image
    cv::Mat img = cv::imread( colImagePath );

    std::cout << "image size = " << img.size( ) << '\n';
    std::cout << "image channels = " << img.channels( ) << '\n';

    // Splitting and merging channels
    cv::Mat imgChannels[ 3 ];
    split( img, imgChannels );

    showMat( imgChannels[ 0 ], "Channel 0", false, 1 );
    showMat( imgChannels[ 1 ], "Channel 1", false, 1 );
    showMat( imgChannels[ 2 ], "Channel 2", false, 1 );
    showMat( img, "Full image", true, 1 );

    // Manipulating color pixels
    cv::Mat colTestImageZero = cv::imread( imagePath, 1 );

    showMat( colTestImageZero, "Color Test Image", false, 5 );

    std::cout << colTestImageZero.at< cv::Vec3b >( 0, 0 ) << '\n';

    colTestImageZero.at< cv::Vec3b >( 0, 0 ) = cv::Vec3b( 0, 255, 255 );
    showMat( colTestImageZero, "Color Test Image 0,0 Mod", false, 5 );

    colTestImageZero.at< cv::Vec3b >( 1, 1 ) = cv::Vec3b( 255, 255, 0 );
    showMat( colTestImageZero, "Color Test Image 1,1 Mod", false, 5 );

    colTestImageZero.at< cv::Vec3b >( 2, 2 ) = cv::Vec3b( 255, 0, 255 );
    showMat( colTestImageZero, "Color Test Image 2,2 Mod", false, 5 );

    colTestImageZero( cv::Range( 0, 3 ), cv::Range( 0, 3 ) )
        .setTo( cv::Scalar( 255, 0, 0 ) );
    colTestImageZero( cv::Range( 3, 6 ), cv::Range( 0, 3 ) )
        .setTo( cv::Scalar( 0, 255, 0 ) );
    colTestImageZero( cv::Range( 6, 9 ), cv::Range( 0, 3 ) )
        .setTo( cv::Scalar( 0, 0, 255 ) );

    showMat( colTestImageZero, "Color Test Image ROI Mod", true, 5 );

    // Images with alpha
    const std::string pantImagePath = "../../../images/panther.png";

    // Read the image
    // Note that we are passing flag = -1 while reading the image ( it will read
    // the image as is)
    cv::Mat imgPNG = cv::imread( pantImagePath, -1 );
    std::cout << "image size = " << imgPNG.size( ) << '\n';
    std::cout << "number of channels = " << imgPNG.channels( ) << '\n';

    cv::Mat imgBGR;
    cv::Mat imgPNGChannels[ 4 ];
    split( imgPNG, imgPNGChannels );

    cv::merge( imgPNGChannels, 3, imgBGR );

    cv::Mat imgMask = imgPNGChannels[ 3 ];

    showMat( imgBGR, "Panther RGB", false, 1 );
    showMat( imgMask, "Panther Mask", true, 1 );

    return 0;
}