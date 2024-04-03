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
constexpr double inScaleFactor = 1.0;
constexpr float confidenceThreshold = 0.4f;
const cv::Scalar meanVal( 104.0, 177.0, 123.0 );

const std::string caffeConfigFile = MODELS_ROOT + "/deploy.prototxt";
const std::string caffeWeightFile =
    MODELS_ROOT + "/res10_300x300_ssd_iter_140000_fp16.caffemodel";

const std::string tensorflowConfigFile =
    MODELS_ROOT + "/opencv_face_detector.pbtxt";
const std::string tensorflowWeightFile =
    MODELS_ROOT + "/opencv_face_detector_uint8.pb";

void detectFaceOpenCVDNN( cv::dnn::Net net, cv::Mat& frameOpenCVDNN )
{
    const int frameHeight = frameOpenCVDNN.rows;
    const int frameWidth = frameOpenCVDNN.cols;
    const cv::Mat inputBlob =
        cv::dnn::blobFromImage( frameOpenCVDNN,
                                inScaleFactor,
                                cv::Size( inWidth, inHeight ),
                                meanVal,
                                false,
                                false );

    net.setInput( inputBlob, "data" );
    cv::Mat detection = net.forward( "detection_out" );

    cv::Mat detectionMat( detection.size[ 2 ],
                          detection.size[ 3 ],
                          CV_32F,
                          detection.ptr< float >( ) );

    for ( int i = 0; i < detectionMat.rows; i++ )
    {
        const float confidence = detectionMat.at< float >( i, 2 );

        if ( confidence > confidenceThreshold )
        {
            const int x1 =
                static_cast< int >( detectionMat.at< float >( i, 3 ) *
                                    static_cast< float >( frameWidth ) );
            const int y1 =
                static_cast< int >( detectionMat.at< float >( i, 4 ) *
                                    static_cast< float >( frameHeight ) );
            const int x2 =
                static_cast< int >( detectionMat.at< float >( i, 5 ) *
                                    static_cast< float >( frameWidth ) );
            const int y2 =
                static_cast< int >( detectionMat.at< float >( i, 6 ) *
                                    static_cast< float >( frameHeight ) );

            cv::rectangle( frameOpenCVDNN,
                           cv::Point( x1, y1 ),
                           cv::Point( x2, y2 ),
                           cv::Scalar( 0, 255, 0 ),
                           frameHeight / 150,
                           8 );
        }
    }
}

#define CAFFE

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
#ifdef CAFFE
    cv::dnn::Net net =
        cv::dnn::readNetFromCaffe( caffeConfigFile, caffeWeightFile );
#else
    cv::dnn::Net net = cv::dnn::readNetFromTensorflow( tensorflowWeightFile,
                                                       tensorflowConfigFile );
#endif

    cv::Mat img = cv::imread( IMAGES_ROOT + "/man.jpg" );
    detectFaceOpenCVDNN( net, img );

    showMat( img, "Face", true );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}