#include <GUI.h>
#include <macros.h>

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/opencv.hpp>
IGNORE_WARNINGS_POP

// STD includes
#include <iostream>

const std::string IMAGES_ROOT = "C:/images";
const std::string RESULTS_ROOT = "C:/images/results";

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    std::string imagePath = IMAGES_ROOT + "/IDCard-Satya.png";
    cv::Mat image = cv::imread( imagePath, cv::IMREAD_COLOR );

    showMat( image, "Input image", false );

    cv::QRCodeDetector qrDecoder;

    cv::Mat bbox, rectifiedImage;
    // std::vector< cv::Point2f > boxPoints;
    std::string result =
        qrDecoder.detectAndDecode( image, bbox, rectifiedImage );

    if ( ! result.empty( ) )
    {
        cv::Mat img = image.clone( );

        std::cout << "Barcode: " << result << "\n";

        std::cout << bbox.size( );

        const auto cols = bbox.cols;

        for ( int i = 0; i < cols; i++ )
        {
            const auto& pt1f = bbox.at< cv::Point2f >( 0, i );
            const auto& pt2f = bbox.at< cv::Point2f >( 0, ( i + 1 ) % cols );

            cv::Point2i pt1( static_cast< int32_t >( pt1f.x ),
                             static_cast< int32_t >( pt1f.y ) );

            cv::Point2i pt2( static_cast< int32_t >( pt2f.x ),
                             static_cast< int32_t >( pt2f.y ) );

            cv::line( img, pt1, pt2, cv::Scalar( 255, 0, 0 ), 3 );
        }

        /*const auto numPoints = boxPoints.size( );
        for ( size_t i = 0; i < boxPoints.size( ); i++ )
        {
            const auto& pt1f = boxPoints[ i ];
            const auto& pt2f = boxPoints[ ( i + 1 ) % numPoints ];

            cv::Point2i pt1( static_cast< int32_t >( pt1f.x ),
                             static_cast< int32_t >( pt1f.y ) );

            cv::Point2i pt2( static_cast< int32_t >( pt2f.x ),
                             static_cast< int32_t >( pt2f.y ) );

            cv::line( img, pt1, pt2, cv::Scalar( 255, 0, 0 ), 3 );
        }*/

        showMat( img, "Result" );
    }

    showMat( rectifiedImage, "Rectified", true );

    return 0;
}