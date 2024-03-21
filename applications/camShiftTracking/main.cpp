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
    const std::string filename = IMAGES_ROOT + "/face1.mp4";
    cv::VideoCapture cap( filename );

    cv::Mat frame;
    cap >> frame;

    // Detect faces in the image
    cv::CascadeClassifier faceCascade;
    cv::String faceCascadePath =
        IMAGES_ROOT + "/models/haarcascade_frontalface_default.xml";

    if ( ! faceCascade.load( faceCascadePath ) )
    {
        std::cout << "--(!)Error loading face cascade\n";
    }

    cv::Mat frameGray;
    cv::cvtColor( frame, frameGray, cv::COLOR_BGR2GRAY );

    std::vector< cv::Rect > faces;
    faceCascade.detectMultiScale( frameGray, faces, 1.3, 5 );

    int x = faces[ 0 ].x;
    int y = faces[ 0 ].y;
    int w = faces[ 0 ].width;
    int h = faces[ 0 ].height;

    cv::Rect currWindow = cv::Rect( x, y, w, h );

    cv::Mat roiObject;

    // get the face region from the frame
    frame( currWindow ).copyTo( roiObject );
    cv::Mat hsvObject;
    cv::cvtColor( roiObject, hsvObject, cv::COLOR_BGR2HSV );

    // Get the mask for calculating histogram of the object and
    // also remove noise
    cv::Mat mask;
    cv::inRange(
        hsvObject, cv::Scalar( 0, 50, 50 ), cv::Scalar( 180, 256, 256 ), mask );

    // Split the image into channels for finding the histogram
    std::vector< cv::Mat > channels( 3 );
    cv::split( hsvObject, channels );

    showMat( mask, "Mask of ROI", false );
    showMat( roiObject, "ROI", true );

    cv::Mat histObject;

    // Initialize parameters for histogram
    int histSize = 180;
    float range[] = { 0, 179 };
    const float* ranges[] = { range };

    // Find the histogram and normalize it to have values
    // between 0 to 255
    cv::calcHist( channels.data( ),
                  1,
                  nullptr,
                  mask,
                  histObject,
                  1,
                  &histSize,
                  ranges,
                  true,
                  false );

    cv::normalize( histObject, histObject, 0, 255, cv::NORM_MINMAX );

    // We will process only first 5 frames
    int count = 0;
    cv::Mat hsv, backProjectImage, frameClone;

    while ( true )
    {
        // Read frame
        cap >> frame;

        if ( frame.empty( ) )
        {
            break;
        }

        // Convert to hsv color space
        cv::cvtColor( frame, hsv, cv::COLOR_BGR2HSV );
        cv::split( hsv, channels );

        // find the back projected image with the histogram obtained earlier
        cv::calcBackProject( channels.data( ),
                             1,
                             nullptr,
                             histObject,
                             backProjectImage,
                             ranges );

        showMat( backProjectImage, "Back Projected Image", false );

        // Compute the new window using mean shift in the present frame
        cv::RotatedRect rotatedWindow = CamShift(
            backProjectImage,
            currWindow,
            cv::TermCriteria(
                cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 10, 1 ) );

        // Get the rotatedWindow vertices
        cv::Point2f rotatedWindowVertices[ 4 ];
        rotatedWindow.points( rotatedWindowVertices );

        // Display the frame with the tracked location of face
        frameClone = frame.clone( );

        rectangle( frameClone,
                   cv::Point( currWindow.x, currWindow.y ),
                   cv::Point( currWindow.x + currWindow.width,
                              currWindow.y + currWindow.height ),
                   cv::Scalar( 255, 0, 0 ),
                   2,
                   cv::LINE_AA );
        // Display the rotated rectangle with the orientation information
        for ( int i = 0; i < 4; i++ )
        {
            line( frameClone,
                  rotatedWindowVertices[ i ],
                  rotatedWindowVertices[ ( i + 1 ) % 4 ],
                  cv::Scalar( 0, 255, 0 ),
                  2,
                  cv::LINE_AA );
        }

        showMat( frameClone, "CAM Shift Object Tracking Demo", false, 1, 100 );

        count += 2;
        if ( count == 10 )
        {
           // break;
        }
    }

    // Clean up
    cap.release( );
    cv::destroyAllWindows( );

    return 0;
}