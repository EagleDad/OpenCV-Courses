#include <GUI.h>
#include <macros.h>

struct Trajectory;
// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
IGNORE_WARNINGS_POP

// STD includes
#include <iostream>

const std::string IMAGES_ROOT = "C:/images";
const std::string RESULTS_ROOT = "C:/images/results";

constexpr int SMOOTHING_RADIUS =
    50; // In frames. The larger the more stable the
        // video, but less reactive to sudden panning

struct TransformParam
{
    TransformParam( ) = default;

    TransformParam( double _dx, double _dy, double _da )
    {
        dx = _dx;
        dy = _dy;
        da = _da;
    }

    double dx;
    double dy;
    double da; // angle

    void getTransform( cv::Mat& T ) const
    {
        // Reconstruct transformation matrix accordingly to new values
        T.at< double >( 0, 0 ) = cos( da );
        T.at< double >( 0, 1 ) = -sin( da );
        T.at< double >( 1, 0 ) = sin( da );
        T.at< double >( 1, 1 ) = cos( da );

        T.at< double >( 0, 2 ) = dx;
        T.at< double >( 1, 2 ) = dy;
    }
};

struct Trajectory
{
    Trajectory( ) = default;

    Trajectory( double _x, double _y, double _a )
    {
        x = _x;
        y = _y;
        a = _a;
    }

    double x;
    double y;
    double a; // angle
};

auto cumsum( const std::vector< TransformParam >& transforms )
{
    std::vector< Trajectory > trajectory; // trajectory at all frames
    // Accumulated frame to frame transform
    double a = 0;
    double x = 0;
    double y = 0;

    for ( size_t i = 0; i < transforms.size( ); i++ )
    {
        x += transforms[ i ].dx;
        y += transforms[ i ].dy;
        a += transforms[ i ].da;

        trajectory.emplace_back( x, y, a );
    }

    return trajectory;
}

auto smooth( const std::vector< Trajectory >& trajectory, int radius )
{
    std::vector< Trajectory > smoothed_trajectory;
    for ( int i = 0; i < static_cast< int >( trajectory.size( ) ); i++ )
    {
        double sum_x = 0;
        double sum_y = 0;
        double sum_a = 0;
        int count = 0;

        for ( int j = -radius; j <= radius; j++ )
        {
            if ( i + j >= 0 &&
                 i + j < static_cast< int >( trajectory.size( ) ) )
            {
                sum_x += trajectory[ static_cast< size_t >( i + j ) ].x;
                sum_y += trajectory[ static_cast< size_t >( i + j ) ].y;
                sum_a += trajectory[ static_cast< size_t >( i + j ) ].a;

                count++;
            }
        }

        double avg_a = sum_a / count;
        double avg_x = sum_x / count;
        double avg_y = sum_y / count;

        smoothed_trajectory.emplace_back( avg_x, avg_y, avg_a );
    }

    return smoothed_trajectory;
}

void fixBorder( cv::Mat& frame_stabilized )
{
    cv::Mat T = cv::getRotationMatrix2D(
        cv::Point2f( static_cast< float >( frame_stabilized.cols ) / 2.0f,
                     static_cast< float >( frame_stabilized.rows ) / 2.0f ),
        0,
        1.04 );

    cv::warpAffine(
        frame_stabilized, frame_stabilized, T, frame_stabilized.size( ) );
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    // Read input video
    cv::VideoCapture cap( IMAGES_ROOT + "/video.mp4" );

    // Get frame count
    int n_frames = static_cast< int >( cap.get( cv::CAP_PROP_FRAME_COUNT ) );

    // Get width and height of video stream
    int w = static_cast< int >( cap.get( cv::CAP_PROP_FRAME_WIDTH ) );
    int h = static_cast< int >( cap.get( cv::CAP_PROP_FRAME_HEIGHT ) );

    // Get frames per second (fps)
    double fps = cap.get( cv::CAP_PROP_FPS );

    // Set up output video
    cv::VideoWriter out( RESULTS_ROOT + "/video_out.avi",
                         cv::VideoWriter::fourcc( 'M', 'J', 'P', 'G' ),
                         fps,
                         cv::Size( 2 * w, h ) );

    // Define variable for storing frames
    cv::Mat curr, curr_gray;
    cv::Mat prev, prev_gray;

    // Read first frame
    cap >> prev;

    // Convert frame to grayscale
    cv::cvtColor( prev, prev_gray, cv::COLOR_BGR2GRAY );

    // Pre-define transformation-store array
    std::vector< TransformParam > transforms;

    cv::Mat last_T;

    for ( int i = 1; i < n_frames - 1; i++ )
    {
        // Vector from previous and current feature points
        std::vector< cv::Point2f > prev_pts, curr_pts;

        // Detect features in previous frame
        cv::goodFeaturesToTrack( prev_gray, prev_pts, 200, 0.01, 30 );

        // Read next frame
        bool success = cap.read( curr );

        if ( ! success )
        {
            break;
        }

        // Convert to grayscale
        cv::cvtColor( curr, curr_gray, cv::COLOR_BGR2GRAY );

        // Calculate optical flow (i.e. track feature points)
        std::vector< uchar > status;
        std::vector< float > err;

        cv::calcOpticalFlowPyrLK(
            prev_gray, curr_gray, prev_pts, curr_pts, status, err );

        // Filter only valid points
        auto prev_it = prev_pts.begin( );
        auto curr_it = curr_pts.begin( );

        for ( size_t k = 0; k < status.size( ); k++ )
        {
            if ( status[ k ] )
            {
                prev_it++;
                curr_it++;
            }
            else
            {
                prev_it = prev_pts.erase( prev_it );
                curr_it = curr_pts.erase( curr_it );
            }
        }
        // Find transformation matrix
        cv::Mat T = cv::estimateAffinePartial2D( prev_pts, curr_pts );

        // In rare cases no transform is found.
        // We'll just use the last known good transform.
        if ( T.data == nullptr )
        {
            last_T.copyTo( T );
        }

        T.copyTo( last_T );

        // Extract translation
        double dx = T.at< double >( 0, 2 );
        double dy = T.at< double >( 1, 2 );

        // Extract rotation angle
        double da = atan2( T.at< double >( 1, 0 ), T.at< double >( 0, 0 ) );

        // Store transformation
        transforms.emplace_back( dx, dy, da );

        // Move to next frame
        curr_gray.copyTo( prev_gray );

        std::cout << "Frame: " << i << "/" << n_frames
                  << " -  Tracked points : " << prev_pts.size( ) << '\n';
    }

    // Compute trajectory using cumulative sum of transformations
    std::vector< Trajectory > trajectory = cumsum( transforms );

    // Smooth trajectory using moving average filter
    std::vector< Trajectory > smoothed_trajectory =
        smooth( trajectory, SMOOTHING_RADIUS );

    std::vector< TransformParam > transforms_smooth;

    for ( size_t i = 0; i < transforms.size( ); i++ )
    {
        // Calculate difference in smoothed_trajectory and trajectory
        double diff_x = smoothed_trajectory[ i ].x - trajectory[ i ].x;
        double diff_y = smoothed_trajectory[ i ].y - trajectory[ i ].y;
        double diff_a = smoothed_trajectory[ i ].a - trajectory[ i ].a;

        // Calculate newer transformation array
        double dx = transforms[ i ].dx + diff_x;
        double dy = transforms[ i ].dy + diff_y;
        double da = transforms[ i ].da + diff_a;

        transforms_smooth.emplace_back( dx, dy, da );
    }

    cap.set( cv::CAP_PROP_POS_FRAMES, 0 );

    cv::Mat T( 2, 3, CV_64F );
    cv::Mat frame, frame_stabilized, frame_out;

    for ( int i = 0; i < n_frames - 1; i++ )
    {
        bool success = cap.read( frame );

        if ( ! success )
        {
            break;
        }

        // Extract transform from translation and rotation angle.
        transforms_smooth[ static_cast< size_t >( i ) ].getTransform( T );

        // Apply affine wrapping to the given frame
        cv::warpAffine( frame, frame_stabilized, T, frame.size( ) );

        // Scale image to remove black border artifact
        fixBorder( frame_stabilized );

        // Now draw the original and stabled side by side for coolness
        cv::hconcat( frame, frame_stabilized, frame_out );

        // If the image is too big, resize it.
        if ( frame_out.cols > 1920 )
        {
            cv::resize( frame_out,
                        frame_out,
                        cv::Size( frame_out.cols / 2, frame_out.rows / 2 ) );
        }

        // imshow("Before and After", frame_out);
        out.write( frame_out );
        // waitKey(10);
    }

    // Release video
    cap.release( );
    out.release( );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}