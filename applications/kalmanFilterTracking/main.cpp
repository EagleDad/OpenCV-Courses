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

// Compare rectangles
bool rectAreaComparator( cv::Rect& r1, cv::Rect& r2 )
{
    return r1.area( ) < r2.area( );
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    // Colors for display
    cv::Scalar blue( 255, 128, 0 );
    cv::Scalar red( 64, 64, 255 );

    // Initialize hog descriptor for people detection
    cv::HOGDescriptor hog( cv::Size( 64, 128 ),      // winSize
                           cv::Size( 16, 16 ),       // blocksize
                           cv::Size( 8, 8 ),         // blockStride,
                           cv::Size( 8, 8 ),         // cellSize,
                           9,                        // nbins,
                           0,                        // derivAperture,
                           -1,                       // winSigma,
                           cv::HOGDescriptor::L2Hys, // histogramNormType,
                           0.2,                      // L2HysThresh,
                           1,                        // gammal correction,
                           64,                       // nlevels=64
                           0 );                      // signedGradient

    // Setup people detector.
    std::vector< float > svmDetector = hog.getDefaultPeopleDetector( );
    hog.setSVMDetector( svmDetector );

    float hitThreshold = 0.0f;
    cv::Size winStride = cv::Size( 8, 8 );
    cv::Size padding = cv::Size( 32, 32 );
    float scale = 1.05f;
    float finalThreshold = 1.0f;
    bool useMeanshiftGrouping = false;
    std::vector< double > weights;

    // Vector for storing objects rectangles
    std::vector< cv::Rect > objects;

    // Load video
    cv::VideoCapture cap( IMAGES_ROOT + "/boy-walking.mp4" );

    // Confirm video is open
    if ( ! cap.isOpened( ) )
    {
        std::cerr << "Unable to read video" << '\n';
    }

    // Variables for storing frames
    cv::Mat frame, frameDisplayDetection, frameDisplay, output;

    // Specify Kalman filter type
    int type = CV_32F;

    // Initialize Kalman filter.
    // Internal state has 6 elements (x, y, width, vx, vy, vw)
    // Measurement has 3 elements (x, y, width ).
    // Note: Height = 2 x width, so it is not part of the state
    // or measurement.
    cv::KalmanFilter KF( 6, 3, 0, type );

    cv::setIdentity( KF.transitionMatrix );

    std::cout << KF.transitionMatrix << '\n';

    cv::setIdentity( KF.measurementMatrix );

    std::cout << KF.measurementMatrix << '\n';

    // Variable to store detected x, y and w
    cv::Mat measurement = cv::Mat::zeros( 3, 1, type );

    // Variables to store detected object and tracked object
    cv::Rect objectTracked, objectDetected;

    // Variables to store results of the predict and update ( a.k.a correct step
    // ).
    cv::Mat updatedMeasurement, predictedMeasurement;

    // Variable to indicate measurement was updated.
    bool measurementWasUpdated = false;

    // Timing variable
    double ticks { }, preTicks { };

    // Read frames until object is detected for the first time
    while ( cap.read( frame ) )
    {
        // Detect object
        hog.detectMultiScale( frame,
                              objects,
                              weights,
                              hitThreshold,
                              winStride,
                              padding,
                              scale,
                              finalThreshold,
                              useMeanshiftGrouping );

        // Update timer
        ticks = static_cast< double >( cv::getTickCount( ) );

        if ( ! objects.empty( ) )
        {
            // First object detected
            objectDetected =
                *std::ranges::max_element( objects, rectAreaComparator );

            // Update measurement
            measurement.at< float >( 0 ) =
                static_cast< float >( objectDetected.x );
            measurement.at< float >( 1 ) =
                static_cast< float >( objectDetected.y );
            measurement.at< float >( 2 ) =
                static_cast< float >( objectDetected.width );

            // Update state. Note x, y, w are set to measured values.
            // vx = vy = vw because we have no idea about the velocities
            // yet.
            KF.statePost.at< float >( 0 ) = measurement.at< float >( 0 );
            KF.statePost.at< float >( 1 ) = measurement.at< float >( 1 );
            KF.statePost.at< float >( 2 ) = measurement.at< float >( 2 );
            KF.statePost.at< float >( 3 ) = 0;
            KF.statePost.at< float >( 4 ) = 0;
            KF.statePost.at< float >( 5 ) = 0;

            // Set diagonal values for covariance matrices.
            // processNoiseCov is Q
            setIdentity( KF.processNoiseCov, cv::Scalar::all( 1e-2 ) );

            // measurementNoiseCov is R
            setIdentity( KF.measurementNoiseCov, cv::Scalar::all( 1e-2 ) );
            break;
        }

        showMat( frame, "Tracking", false );
    }

    showMat( frame, "Tracking", true );

    // Apply Kalman Filter
    // dt for Transition matrix
    double dt = 0;

    // Random number generator for randomly selecting frames for update
    cv::RNG rng( 0xFFFFFFFF );

    // We will display only first 5 frames
    int count = 0;

    // Loop over all frames
    while ( cap.read( frame ) )
    {
        // Variable for displaying tracking result
        frameDisplay = frame.clone( );

        // Variable for displaying detection result
        frameDisplayDetection = frame.clone( );

        // Update dt for transition matrix.
        // dt = time elapsed.

        preTicks = ticks;
        ticks = static_cast< double >( cv::getTickCount( ) );
        dt = ( ticks - preTicks ) / cv::getTickFrequency( );

        KF.transitionMatrix.at< float >( 3 ) = static_cast< float >( dt );
        KF.transitionMatrix.at< float >( 10 ) = static_cast< float >( dt );
        KF.transitionMatrix.at< float >( 17 ) = static_cast< float >( dt );

        // Kalman filter predict step
        predictedMeasurement = KF.predict( );

        // Clear objects detected in previous frame.
        objects.clear( );

        // Detect objects in current frame
        hog.detectMultiScale( frame,
                              objects,
                              weights,
                              hitThreshold,
                              winStride,
                              padding,
                              scale,
                              finalThreshold,
                              useMeanshiftGrouping );

        // Find largest object
        objectDetected =
            *std::ranges::max_element( objects, rectAreaComparator );

        // Display detected rectangle
        rectangle( frameDisplayDetection, objectDetected, red, 2, 4 );

        // We will update measurements 15% of the time.
        // Frames are randomly chosen.

        if ( bool update = rng.uniform( 0.0, 1.0 ) < 0.15 )
        {
            // Kalman filter update step
            if ( ! objects.empty( ) )
            {
                // Copy x, y, w from the detected rectangle
                measurement.at< float >( 0 ) =
                    static_cast< float >( objectDetected.x );
                measurement.at< float >( 1 ) =
                    static_cast< float >( objectDetected.y );
                measurement.at< float >( 2 ) =
                    static_cast< float >( objectDetected.width );

                // Perform Kalman update step
                updatedMeasurement = KF.correct( measurement );
                measurementWasUpdated = true;
            }
            else
            {
                // Measurement not updated because no object detected
                measurementWasUpdated = false;
            }
        }
        else
        {
            // Measurement not updated
            measurementWasUpdated = false;
        }

        if ( measurementWasUpdated )
        {
            // Use updated measurement if measurement was updated
            objectTracked = cv::Rect(
                static_cast< int >( updatedMeasurement.at< float >( 0 ) ),
                static_cast< int >( updatedMeasurement.at< float >( 1 ) ),
                static_cast< int >( updatedMeasurement.at< float >( 2 ) ),
                2 * static_cast< int >( updatedMeasurement.at< float >( 2 ) ) );
        }
        else
        {
            // If measurement was not updated, use predicted values.
            objectTracked = cv::Rect(
                static_cast< int >( predictedMeasurement.at< float >( 0 ) ),
                static_cast< int >( predictedMeasurement.at< float >( 1 ) ),
                static_cast< int >( predictedMeasurement.at< float >( 2 ) ),
                2 * static_cast< int >(
                        predictedMeasurement.at< float >( 2 ) ) );
        }

        // Draw tracked object
        cv::rectangle( frameDisplay, objectTracked, blue, 2, 4 );

        // Text indicating Tracking or Detection.
        cv::putText( frameDisplay,
                     "Tracking",
                     cv::Point( 20, 40 ),
                     cv::FONT_HERSHEY_SIMPLEX,
                     0.75,
                     blue,
                     2 );
        cv::putText( frameDisplayDetection,
                     "Detection",
                     cv::Point( 20, 40 ),
                     cv::FONT_HERSHEY_SIMPLEX,
                     0.75,
                     red,
                     2 );

        // Concatenate detected result and tracked result vertically
        vconcat( frameDisplayDetection, frameDisplay, output );

        showMat( output, "Tracking", false );

        count += 1;
        /*if ( count == 5 )
            break;*/
    }

    // Clean up
    cv::waitKey( 0 );
    cv::destroyAllWindows( );

    return 0;
}