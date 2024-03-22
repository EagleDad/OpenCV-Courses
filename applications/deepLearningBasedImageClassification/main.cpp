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

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    //
    // Using Caffe models
    //

    std::string protoFile = MODELS_ROOT + "/bvlc_googlenet.prototxt";
    std::string weightFile = MODELS_ROOT + "/bvlc_googlenet.caffemodel";
    std::string filename = IMAGES_ROOT + "/panda.jpg";

    cv::Mat frame = cv::imread( filename );

    std::vector< std::string > classes;

    std::string classFile =
        MODELS_ROOT + "/classification_classes_ILSVRC2012.txt";
    std::ifstream ifs( classFile.c_str( ) );
    std::string line;

    while ( std::getline( ifs, line ) )
    {
        classes.push_back( line );
    }

    float scale = 1.0;
    int inHeight = 224;
    int inWidth = 224;
    bool swapRB = false;
    // The mean that will be subtracted should be defined in the paper of the
    // network
    cv::Scalar mean = cv::Scalar( 104, 117, 123 );

    //! [Read and initialize network]
    cv::dnn::Net net = cv::dnn::readNetFromCaffe( protoFile, weightFile );

    // Process frames.
    cv::Mat blob;
    //! [Create a 4D blob from a frame]
    cv::dnn::blobFromImage( frame,
                            blob,
                            scale,
                            cv::Size( inWidth, inHeight ),
                            mean,
                            swapRB,
                            false );

    //! [Set input blob]
    net.setInput( blob );

    //! [Make forward pass]
    cv::Mat prob = net.forward( );

    //! [Get a class with a highest score]
    cv::Point classIdPoint;
    double confidence;
    cv::minMaxLoc( prob.reshape( 1, 1 ), 0, &confidence, 0, &classIdPoint );
    int classId = classIdPoint.x;

    // Print predicted class.
    std::string label =
        cv::format( "Predicted Class : %s, confidence : %.3f",
                    classes[ static_cast< size_t >( classId ) ].c_str( ),
                    confidence );

    std::cout << label << '\n';

    showMat( frame, "Panda", true );

    //
    // Using Tensorflow models
    //

    weightFile = MODELS_ROOT + "/tensorflow_inception_graph.pb";

    float scale2 = 1.0;
    int inHeight2 = 224;
    int inWidth2 = 224;
    bool swapRB2 = true;
    cv::Scalar mean2 = cv::Scalar( 117, 117, 117 );

    std::vector< std::string > classes2;

    std::string classFile2 =
        MODELS_ROOT + "/imagenet_comp_graph_label_strings.txt";
    std::ifstream ifs2( classFile2.c_str( ) );
    std::string line2;
    while ( getline( ifs2, line2 ) )
    {
        classes2.push_back( line2 );
    }

    //! [Read and initialize network]
    cv::dnn::Net net2 = cv::dnn::readNetFromTensorflow( weightFile );

    // Process frames.
    cv::Mat blob2;
    //! [Create a 4D blob from a frame]
    cv::dnn::blobFromImage( frame,
                            blob2,
                            scale2,
                            cv::Size( inWidth2, inHeight2 ),
                            mean2,
                            swapRB2,
                            false );

    //! [Set input blob]
    net2.setInput( blob2 );

    //! [Make forward pass]

    cv::Mat prob2 = net2.forward( );

    //! [Get a class with a highest score]
    cv::Point classIdPoint2;
    double confidence2;
    cv::minMaxLoc( prob2.reshape( 1, 1 ), 0, &confidence2, 0, &classIdPoint2 );
    int classId2 = classIdPoint2.x;

    // Print predicted class.
    label = cv::format( "Predicted Class : %s, confidence : %.3f",
                        classes2[ static_cast< size_t >( classId2 ) ].c_str( ),
                        confidence2 );

    std::cout << label << '\n';

    showMat( frame, "Panda", true );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}