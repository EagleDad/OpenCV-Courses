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
    // Load the cascade classifier from the xml file
    cv::String faceCascadePath =
        IMAGES_ROOT + "/models/haarcascade_frontalface_default.xml";
    cv::CascadeClassifier faceCascade;

    if ( ! faceCascade.load( faceCascadePath ) )
    {
        std::cout << "--(!)Error loading face cascade\n";
        return -1;
    }
    else
    {
        std::cout << "Loaded face cascade successful\n";
    }

    int faceNeighborsMax = 10;
    int neighborStep = 1;

    // Read the image
    cv::Mat frame = cv::imread( IMAGES_ROOT + "/hillary_clinton.jpg" );
    cv::Mat frameGray;
    cv::cvtColor( frame, frameGray, cv::COLOR_BGR2GRAY );

    showMat( frame, "Hillary", true );

    // Perform multi scale detection of faces
    std::vector< cv::Rect > faces;
    int count = 1;
    cv::Mat frameClone;

    for ( int neigh = 1; neigh < faceNeighborsMax;
          neigh = neigh + neighborStep )
    {
        frameClone = frame.clone( );
        faceCascade.detectMultiScale( frameGray, faces, 1.2, neigh );
        for ( size_t i = 0; i < faces.size( ); i++ )
        {
            int x = faces[ i ].x;
            int y = faces[ i ].y;
            int w = faces[ i ].width;
            int h = faces[ i ].height;

            cv::rectangle( frameClone,
                           cv::Point( x, y ),
                           cv::Point( x + w, y + h ),
                           cv::Scalar( 255, 0, 0 ),
                           2,
                           4 );
        }

        cv::putText( frameClone,
                     cv::format( "# Neighbors = %d", neigh ),
                     cv::Point( 10, 50 ),
                     cv::FONT_HERSHEY_SIMPLEX,
                     1,
                     cv::Scalar( 0, 0, 255 ),
                     4 );

        showMat(
            frameClone, "Face Detection " + std::to_string( count ), true );
        count += 1;
    }

    cv::String smileCascadePath;
    cv::CascadeClassifier smileCascade;

    smileCascadePath = IMAGES_ROOT + "/models/haarcascade_smile.xml";

    if ( ! smileCascade.load( smileCascadePath ) )
    {
        std::cout << "--(!)Error loading smile cascade\n";
        return -1;
    }
    else
    {
        std::cout << "Loaded smile cascade successful\n";
    }

    faceCascade.detectMultiScale( frameGray, faces, 1.4, 5 );

    cv::Mat faceRoiGray, faceRoiOriginal;
    int x { }, y { }, w { }, h { };

    // Get the face area from the detected face rectangle
    for ( size_t i = 0; i < faces.size( ); i++ )
    {
        x = faces[ i ].x;
        y = faces[ i ].y;
        w = faces[ i ].width;
        h = faces[ i ].height;

        rectangle( frame,
                   cv::Point( x, y ),
                   cv::Point( x + w, y + h ),
                   cv::Scalar( 255, 0, 0 ),
                   2,
                   4 );

        faceRoiGray = frameGray( faces[ i ] );
        faceRoiOriginal = faceRoiGray.clone( );
    }

    int smileNeighborsMax = 90;
    neighborStep = 10;

    int count_plot = 1;

    std::vector< cv::Rect > smile;

    cv::Mat faceRoiClone;

    for ( int neigh = 1; neigh < smileNeighborsMax;
          neigh = neigh + neighborStep )
    {
        frameClone = frame.clone( );
        //-- In each face, detect smile
        smileCascade.detectMultiScale( faceRoiGray, smile, 1.5, neigh );
        faceRoiClone =
            frameClone( cv::Range( y, y + h ), cv::Range( x, x + w ) );

        for ( size_t j = 0; j < smile.size( ); j++ )
        {
            int smileX = smile[ j ].x;
            int smileY = smile[ j ].y;
            int smileW = smile[ j ].width;
            int smileH = smile[ j ].height;

            cv::rectangle( faceRoiClone,
                           cv::Point( smileX, smileY ),
                           cv::Point( smileX + smileW, smileY + smileH ),
                           cv::Scalar( 0, 255, 0 ),
                           2,
                           4 );
        }

        cv::putText( frameClone,
                     cv::format( "# Neighbors = %d", neigh ),
                     cv::Point( 10, 50 ),
                     cv::FONT_HERSHEY_SIMPLEX,
                     1,
                     cv::Scalar( 0, 0, 255 ),
                     4 );

        showMat( frameClone,
                 "Smile Detection " + std::to_string( count_plot ),
                 true );

        count_plot += 1;
    }

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}