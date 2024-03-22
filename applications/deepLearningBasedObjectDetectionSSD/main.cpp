#include <GUI.h>
#include <macros.h>

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
IGNORE_WARNINGS_POP

// STD includes
#include <fstream>
#include <iostream>

const std::string IMAGES_ROOT = "C:/images";
const std::string MODELS_ROOT = "C:/images/models";
const std::string RESULTS_ROOT = "C:/images/results";

constexpr size_t inWidth = 300;
constexpr size_t inHeight = 300;
constexpr double inScaleFactor = 1.0 / 127.5;
constexpr float confidenceThreshold = 0.7f;
const cv::Scalar meanVal( 127.5, 127.5, 127.5 );
std::vector< std::string > classes;

// Detect Objects
cv::Mat detect_objects( cv::dnn::Net net, const cv::Mat& frame )
{
    const cv::Mat inputBlob =
        cv::dnn::blobFromImage( frame,
                                inScaleFactor,
                                cv::Size( inWidth, inHeight ),
                                meanVal,
                                true,
                                false );
    net.setInput( inputBlob );
    cv::Mat detection = net.forward( "detection_out" );
    cv::Mat detectionMat( detection.size[ 2 ],
                          detection.size[ 3 ],
                          CV_32F,
                          detection.ptr< float >( ) );

    return detectionMat;
}

void display_text( cv::Mat& img, std::string text, int x, int y )
{
    // Get text size
    int baseLine { };

    const cv::Size textSize =
        cv::getTextSize( text, cv::FONT_HERSHEY_SIMPLEX, 0.7, 1, &baseLine );

    // Use text size to create a black rectangle
    cv::rectangle( img,
                   cv::Point( x, y - textSize.height - baseLine ),
                   cv::Point( x + textSize.width, y + baseLine ),
                   cv::Scalar( 0, 0, 0 ),
                   -1 );

    // Display text inside the rectangle
    cv::putText( img,
                 text,
                 cv::Point( x, y - 5 ),
                 cv::FONT_HERSHEY_SIMPLEX,
                 0.7,
                 cv::Scalar( 0, 255, 255 ),
                 1,
                 cv::LINE_AA );
}

// Display Objects
void display_objects( cv::Mat& img, cv::Mat objects, float threshold = 0.25 )
{
    // For every detected object
    for ( int i = 0; i < objects.rows; i++ )
    {
        const int classId = static_cast< int >( objects.at< float >( i, 1 ) );
        const float score = objects.at< float >( i, 2 );

        // Recover original coordinates from normalized coordinates
        const int x = static_cast< int >( objects.at< float >( i, 3 ) *
                                          static_cast< float >( img.cols ) );
        const int y = static_cast< int >( objects.at< float >( i, 4 ) *
                                          static_cast< float >( img.rows ) );
        const int w = static_cast< int >( objects.at< float >( i, 5 ) *
                                              static_cast< float >( img.cols ) -
                                          static_cast< float >( x ) );
        const int h = static_cast< int >( objects.at< float >( i, 6 ) *
                                              static_cast< float >( img.rows ) -
                                          static_cast< float >( y ) );

        // Check if the detection is of good quality
        if ( score > threshold )
        {
            display_text(
                img, classes[ static_cast< size_t >( classId ) ], x, y );
            cv::rectangle( img,
                           cv::Point( x, y ),
                           cv::Point( x + w, y + h ),
                           cv::Scalar( 255, 255, 255 ),
                           2 );
        }
    }
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    //
    // Single Shot Multibox Detector
    //
    const std::string configFile =
        MODELS_ROOT + "/ssd_mobilenet_v2_coco_2018_03_29.pbtxt";

    const std::string modelFile =
        MODELS_ROOT +
        "/ssd_mobilenet_v2_coco_2018_03_29/frozen_inference_graph.pb";

    const std::string classFile = MODELS_ROOT + "/coco_class_labels.txt";

    //
    // Read Tensorflow Model
    //

    cv::dnn::Net net = cv::dnn::readNetFromTensorflow( modelFile, configFile );

    //
    // Check Class Labels
    //

    std::ifstream ifs( classFile.c_str( ) );
    std::string line;
    while ( std::getline( ifs, line ) )
    {
        classes.push_back( line );
    }

    //
    // Results
    //
    cv::Mat img, objects;

    // Street

    img = cv::imread( IMAGES_ROOT + "/street.jpg" );
    objects = detect_objects( net, img );

    display_objects( img, objects );

    showMat( img, "Street", true );

    // Baseball
    img = cv::imread( IMAGES_ROOT + "/baseball.jpg" );
    objects = detect_objects( net, img );
    display_objects( img, objects );
    showMat( img, "Baseball", true );

    // Soccer
    img = cv::imread( IMAGES_ROOT + "/soccer.jpg" );
    objects = detect_objects( net, img );
    display_objects( img, objects );
    showMat( img, "Soccer", true );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}