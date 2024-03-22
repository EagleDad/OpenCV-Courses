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

// Initialize the parameters
float objectnessThreshold = 0.5f; // Objectness threshold
float confThreshold = 0.5f;       // Confidence threshold
float nmsThreshold = 0.4f;        // Non-maximum suppression threshold
int inpWidth = 416;               // Width of network's input image
int inpHeight = 416;              // Height of network's input image
std::vector< std::string > classes;

// Get the names of the output layers
auto getOutputsNames( const cv::dnn::Net& net )
{
    static std::vector< cv::String > names;

    if ( names.empty( ) )
    {
        // Get the indices of the output layers, i.e. the layers with
        // unconnected outputs
        const std::vector< int > outLayers = net.getUnconnectedOutLayers( );

        // get the names of all the layers in the network
        const std::vector< cv::String > layersNames = net.getLayerNames( );

        // Get the names of the output layers in names
        names.resize( outLayers.size( ) );

        for ( size_t i = 0; i < outLayers.size( ); ++i )
        {
            names[ i ] =
                layersNames[ static_cast< size_t >( outLayers[ i ] ) - 1 ];
        }
    }
    return names;
}

// Draw the predicted bounding box
void drawPred( int classId, float conf, int left, int top, int right,
               int bottom, cv::Mat& frame )
{
    // Draw a rectangle displaying the bounding box
    cv::rectangle( frame,
                   cv::Point( left, top ),
                   cv::Point( right, bottom ),
                   cv::Scalar( 255, 178, 50 ),
                   3 );

    // Get the label for the class name and its confidence
    std::string label = cv::format( "%.2f", conf );

    if ( ! classes.empty( ) )
    {
        CV_Assert( classId < ( int )classes.size( ) );
        label = classes[ static_cast< size_t >( classId ) ] + ":" + label;
    }

    // Display the label at the top of the bounding box
    int baseLine { };
    const cv::Size labelSize =
        cv::getTextSize( label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine );
    top = std::max( top, labelSize.height );

    cv::rectangle(
        frame,
        cv::Point( left,
                   static_cast< int >( static_cast< double >( top ) -
                                       std::round( 1.5 * labelSize.height ) ) ),
        cv::Point( static_cast< int >( static_cast< double >( left ) +
                                       std::round( 1.5 * labelSize.width ) ),
                   top + baseLine ),
        cv::Scalar( 255, 255, 255 ),
        cv::FILLED );

    cv::putText( frame,
                 label,
                 cv::Point( left, top ),
                 cv::FONT_HERSHEY_SIMPLEX,
                 0.75,
                 cv::Scalar( 0, 0, 0 ),
                 1 );
}

// Remove the bounding boxes with low confidence using non-maxima suppression
void postprocess( cv::Mat& frame, const std::vector< cv::Mat >& outs )
{
    std::vector< int > classIds;
    std::vector< float > confidences;
    std::vector< cv::Rect > boxes;

    for ( size_t i = 0; i < outs.size( ); ++i )
    {
        // Scan through all the bounding boxes output from the network and keep
        // only the ones with high confidence scores. Assign the box's class
        // label as the class with the highest score for the box.
        float* data = reinterpret_cast< float* >( outs[ i ].data );

        for ( int j = 0; j < outs[ i ].rows; ++j, data += outs[ i ].cols )
        {
            // Get the objectness score for each box
            const float objectness = outs[ i ].at< float >( j, 4 );

            if ( objectness > objectnessThreshold )
            {
                cv::Mat scores =
                    outs[ i ].row( j ).colRange( 5, outs[ i ].cols );
                cv::Point classIdPoint;
                double confidence;
                // Get the value and location of the maximum score
                cv::minMaxLoc( scores, 0, &confidence, 0, &classIdPoint );

                if ( confidence > confThreshold )
                {
                    const int centerX = static_cast< int >(
                        data[ 0 ] * static_cast< float >( frame.cols ) );
                    const int centerY = static_cast< int >(
                        data[ 1 ] * static_cast< float >( frame.rows ) );
                    const int width = static_cast< int >(
                        data[ 2 ] * static_cast< float >( frame.cols ) );
                    const int height = static_cast< int >(
                        data[ 3 ] * static_cast< float >( frame.rows ) );
                    const int left = centerX - width / 2;
                    const int top = centerY - height / 2;

                    classIds.push_back( classIdPoint.x );
                    confidences.push_back( static_cast< float >( confidence ) );
                    boxes.emplace_back( left, top, width, height );
                }
            }
        }
    }

    // Perform non maximum suppression to eliminate redundant overlapping boxes
    // with lower confidences
    std::vector< int > indices;
    cv::dnn::NMSBoxes(
        boxes, confidences, confThreshold, nmsThreshold, indices );

    for ( size_t i = 0; i < indices.size( ); ++i )
    {
        const int idx = indices[ i ];
        const cv::Rect box = boxes[ static_cast< size_t >( idx ) ];
        drawPred( classIds[ static_cast< size_t >( idx ) ],
                  confidences[ static_cast< size_t >( idx ) ],
                  box.x,
                  box.y,
                  box.x + box.width,
                  box.y + box.height,
                  frame );
    }
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    // Load names of classes
    const std::string classesFile = MODELS_ROOT + "/coco.names";
    std::ifstream ifs( classesFile.c_str( ) );
    std::string line;

    while ( std::getline( ifs, line ) )
    {
        classes.push_back( line );
    }

    // Give the configuration and weight files for the model
    const cv::String modelConfiguration = MODELS_ROOT + "/yolov3.cfg";
    const cv::String modelWeights = MODELS_ROOT + "/yolov3.weights";

    // Load the network
    cv::dnn::Net net =
        cv::dnn::readNetFromDarknet( modelConfiguration, modelWeights );

    std::string imagePath = IMAGES_ROOT + "/bird.jpg";
    cv::Mat frame = cv::imread( imagePath );

    // Create a 4D blob from a frame.
    cv::Mat blob;
    cv::dnn::blobFromImage( frame,
                            blob,
                            1 / 255.0,
                            cv::Size( inpWidth, inpHeight ),
                            cv::Scalar( 0, 0, 0 ),
                            true,
                            false );

    // Sets the input to the network
    net.setInput( blob );

    // Runs the forward pass to get output of the output layers
    std::vector< cv::Mat > outs;
    net.forward( outs, getOutputsNames( net ) );

    // Remove the bounding boxes with low confidence
    postprocess( frame, outs );

    // Put efficiency information. The function getPerfProfile returns the
    // overall time for inference(t) and the timings for each of the layers(in
    // layersTimes)
    std::vector< double > layersTimes;
    double freq = cv::getTickFrequency( ) / 1000;
    double t =
        static_cast< double >( net.getPerfProfile( layersTimes ) ) / freq;
    std::string label = cv::format( "Inference time for a frame : %.2f ms", t );
    cv::putText( frame,
                 label,
                 cv::Point( 0, 15 ),
                 cv::FONT_HERSHEY_SIMPLEX,
                 0.5,
                 cv::Scalar( 0, 0, 255 ) );

    std::cout << label << '\n';

    showMat( frame, "Bird", true );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}