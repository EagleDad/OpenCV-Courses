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
    const int POSE_PAIRS[ 14 ][ 2 ] = { { 0, 1 },
                                        { 1, 2 },
                                        { 2, 3 },
                                        { 3, 4 },
                                        { 1, 5 },
                                        { 5, 6 },
                                        { 6, 7 },
                                        { 1, 14 },
                                        { 14, 8 },
                                        { 8, 9 },
                                        { 9, 10 },
                                        { 14, 11 },
                                        { 11, 12 },
                                        { 12, 13 } };

    //
    // Load a Caffe Model
    //
    std::string protoFile = MODELS_ROOT + "/mpi.prototxt";
    std::string weightsFile = MODELS_ROOT + "/pose_iter_160000.caffemodel";

    int nPoints = 15;
    cv::dnn::Net net = cv::dnn::readNetFromCaffe( protoFile, weightsFile );

    //
    // Read Image
    //
    cv::Mat img = cv::imread( IMAGES_ROOT + "/man.jpg" );
    cv::Mat imgOrig = img.clone( );
    cv::cvtColor( img, img, cv::COLOR_BGR2RGB );
    showMat( imgOrig, "Man", true );
    int inWidth = img.cols;
    int inHeight = img.rows;

    //
    // Convert image to blob
    //
    cv::Size netInputSize = cv::Size( 368, 368 );
    cv::Mat inpBlob = cv::dnn::blobFromImage(
        img, 1.0 / 255, netInputSize, cv::Scalar( 0, 0, 0 ), false, false );
    net.setInput( inpBlob );

    //
    // Run Inference (forward pass)
    //
    // Forward pass
    cv::Mat output = net.forward( );

    int H = output.size[ 2 ];
    int W = output.size[ 3 ];

    // Display probability maps
    for ( int i = 0; i < nPoints; i++ )
    {
        cv::Mat probMap( H, W, CV_32F, output.ptr( 0, i ) );
        cv::Mat displayMap;
        cv::resize( probMap,
                    displayMap,
                    cv::Size( inWidth, inHeight ),
                    cv::INTER_LINEAR );

        showMat( displayMap, "Prop " + std::to_string( i ), true );
    }

    //
    // Extract points
    //
    float thresh = 0.1f;
    int frameWidth = img.cols;
    int frameHeight = img.rows;

    cv::Mat frameCopy = imgOrig.clone( );

    // find the position of the body parts
    std::vector< cv::Point > points( static_cast< size_t >( nPoints ) );

    for ( int n = 0; n < nPoints; n++ )
    {
        // Probability map of corresponding body's part.
        cv::Mat probMap( H, W, CV_32F, output.ptr( 0, n ) );

        cv::Point2f p( -1, -1 );
        cv::Point maxLoc;
        double prob;
        minMaxLoc( probMap, 0, &prob, 0, &maxLoc );
        if ( prob > thresh )
        {
            p = maxLoc;
            p.x *=
                static_cast< float >( frameWidth ) / static_cast< float >( W );
            p.y *=
                static_cast< float >( frameHeight ) / static_cast< float >( H );

            cv::circle( frameCopy,
                        cv::Point( static_cast< int >( p.x ),
                                   static_cast< int >( p.y ) ),
                        8,
                        cv::Scalar( 0, 255, 255 ),
                        -1 );
            cv::putText( frameCopy,
                         cv::format( "%d", n ),
                         cv::Point( static_cast< int >( p.x ),
                                    static_cast< int >( p.y ) ),
                         cv::FONT_HERSHEY_COMPLEX,
                         1,
                         cv::Scalar( 0, 0, 255 ),
                         2 );
        }
        points[ static_cast< size_t >( n ) ] = p;
    }

    //
    // Display Points & Skeleton
    //
    int nPairs = static_cast< int >( std::size( POSE_PAIRS ) );

    for ( int n = 0; n < nPairs; n++ )
    {
        // lookup 2 connected body/hand parts
        cv::Point2f partA =
            points[ static_cast< size_t >( POSE_PAIRS[ n ][ 0 ] ) ];
        cv::Point2f partB =
            points[ static_cast< size_t >( POSE_PAIRS[ n ][ 1 ] ) ];

        if ( partA.x <= 0 || partA.y <= 0 || partB.x <= 0 || partB.y <= 0 )
        {
            continue;
        }

        line( imgOrig, partA, partB, cv::Scalar( 0, 255, 255 ), 8 );
        cv::circle( imgOrig, partA, 8, cv::Scalar( 0, 0, 255 ), -1 );
        cv::circle( imgOrig, partB, 8, cv::Scalar( 0, 0, 255 ), -1 );
    }

    showMat( frameCopy, "Points", false );
    showMat( imgOrig, "Pose", true );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}