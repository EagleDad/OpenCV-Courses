#include <GUI.h>
#include <macros.h>

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
IGNORE_WARNINGS_POP

// STD includes
#include <iostream>

const std::string IMAGES_ROOT = "C:/images";
const std::string RESULTS_ROOT = "C:/images/results";

cv::Mat displayConnectedComponents( cv::Mat& im )
{
    // Make a copy of the image
    cv::Mat imLabels = im.clone( );

    // First let's find the min and max values in imLabels
    cv::Point minLoc, maxLoc;
    double min, max;

    // The following line finds the min and max pixel values
    // and their locations in an image.
    cv::minMaxLoc( imLabels, &min, &max, &minLoc, &maxLoc );

    // Normalize the image so the min value is 0 and max value is 255.
    imLabels = 255 * ( imLabels - min ) / ( max - min );

    // Convert image to 8-bits
    imLabels.convertTo( imLabels, CV_8U );

    // Apply a color map
    cv::Mat imColorMap;
    applyColorMap( imLabels, imColorMap, cv::COLORMAP_JET );

    return imColorMap;
}

void assignmentPartA( );
void assignmentPartB( );

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    // assignmentPartA( );

    assignmentPartB( );

    // Clean up
    // system( "pause" ); // NOLINT(concurrency-mt-unsafe)
    cv::destroyAllWindows( );

    return 0;
}

void assignmentPartA( )
{
    // Image path
    std::string imagePath = IMAGES_ROOT + "/CoinsA.png";
    // Read image
    // Store it in the variable image
    ///
    /// YOUR CODE HERE
    ///
    // Read the image as color image
    cv::Mat image = cv::imread( imagePath, cv::IMREAD_COLOR );

    // Get a copy of the input image
    cv::Mat imageCopy = image.clone( );

    showMat( image, "Input image", false );

    // Convert image to grayscale
    // Store it in the variable imageGray
    ///
    /// YOUR CODE HERE
    ///
    cv::Mat imageGray;
    cv::cvtColor( image, imageGray, cv::COLOR_BGR2GRAY );
    showMat( imageGray, "Input gray", false );

    // Split cell into channels
    // Store them in variables imageB, imageG, imageR
    ///
    /// YOUR CODE HERE
    cv::Mat channels[ 3 ];
    cv::split( image, channels );

    // The channels are ordered BGR - no need to clone yet
    cv::Mat imageB = channels[ 0 ];
    cv::Mat imageG = channels[ 1 ];
    cv::Mat imageR = channels[ 2 ];

    showMat( imageB, "Channel Blue", false );
    showMat( imageG, "Channel Green", false );
    showMat( imageR, "Channel Red", false );

    // The green channel looks best for doing the segmentation.
    cv::Mat dst;
    cv::Mat imageToThresh = imageG.clone( );

    // Due to the structured background the image might have some noise after
    // segmentation. We perform a median filter to eliminate some noise. The
    // median is an edge preserving filter.
    cv::medianBlur( imageToThresh, imageToThresh, 5 );

    // We know, that the top left corner of the image contains only background
    // information. Let's use a roi in that region in that area to calculate
    // mean and standard deviation of the gray values in that region. The
    // standard deviation gives us some information about the scattering of the
    // values around the mean. This is interesting because of the pattern in the
    // background. We'll use mean + 5 times the deviation as threshold.
    cv::Mat roi = imageToThresh( cv::Rect( 0, 0, 50, 50 ) );
    showMat( roi, "ROI", false, 2 );
    cv::Scalar mean;
    cv::Scalar stdDev;
    cv::meanStdDev( roi, mean, stdDev );
    double thresh = mean[ 0 ] + 5 * stdDev[ 0 ];
    std::cout << "Mean: " << mean[ 0 ] << "\n";
    std::cout << "StdDev: " << stdDev[ 0 ] << "\n";

    std::cout << "Threshold value to use: " << thresh << "\n";

    cv::threshold( imageToThresh,
                   dst,
                   thresh,
                   1,
                   cv::THRESH_BINARY /*| cv::THRESH_OTSU*/ );
    showMat( dst * 255, "Segmented green channel", false );

    // The coins seem to be segmented nicely. There is only some noise in the
    // image. Although some of the objects are quite close to each other.
    // Performing a closing directly might glue them together.
    // We'll perform an erosion first. We do this with a circular shape
    // structure element. We choose a circular shape because the coins are also
    // circular. Using e.g. a rectangular will change the outer shape of our
    // coins. We also use a bigger kernel to make sure the artifact will
    // disappear. In the segmented image we can also see a kind of circular
    // shape at the center bottom position of the image. With a bigger erosion
    // kernel we can get rif of the as well.
    cv::Mat structureElement =
        cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 9, 9 ) );

    showMat( structureElement * 255, "Element for closing", false, 4 );

    cv::Mat eroded;
    cv::morphologyEx( dst,
                      eroded,
                      cv::MORPH_ERODE,
                      structureElement,
                      cv::Point2i( -1, -1 ),
                      1 );

    showMat( eroded * 255, "Eroded objects", false );

    // Since we reduced the size with the erosion, we have to dilate with the
    // same kernel to get the original object size back.

    cv::Mat dilated;
    cv::morphologyEx( eroded,
                      dilated,
                      cv::MORPH_DILATE,
                      structureElement,
                      cv::Point2i( -1, -1 ),
                      1 );

    dilated *= 255;

    showMat( dilated, "Opened objects", false );

    // Now it is time to detect the blobs. Since we are looking for coins we can
    // expect circular shaped objects. We also know, that we are looking for
    // bright objects, due to the decision for thresholding.
    // Setup SimpleBlobDetector parameters.
    cv::SimpleBlobDetector::Params params;

    // Set color
    params.blobColor = 255;

    // Filter by Area
    params.filterByArea = false;

    // Filter by Circularity
    params.filterByCircularity = true;
    params.minCircularity = 0.8f;

    // Filter by Convexity
    params.filterByConvexity = false;
    params.minConvexity = 0.8f;

    // Filter by Inertia
    params.filterByInertia = false;
    params.minInertiaRatio = 0.8f;

    // Set up detector with params
    cv::Ptr< cv::SimpleBlobDetector > detector =
        cv::SimpleBlobDetector::create( params );

    // kcv::bitwise_not( dilated, dilated );

    std::vector< cv::KeyPoint > keypoints;
    detector->detect( dilated, keypoints );

    std::cout << "Number blobs detected: " << keypoints.size( ) << "\n";

    int x, y;
    int radius;
    double diameter;

    // Lets display the detected blobs
    for ( int i = 0; i < static_cast< int >( keypoints.size( ) ); i++ )
    {
        cv::KeyPoint k = keypoints[ static_cast< size_t >( i ) ];
        cv::Point keyPt;
        keyPt = k.pt;
        x = keyPt.x;
        y = keyPt.y;
        // Mark center
        cv::circle( image, cv::Point( x, y ), 5, cv::Scalar( 255, 0, 0 ), -1 );
        // Get radius of coin
        diameter = k.size;
        radius = static_cast< int >( diameter / 2.0 );
        // Mark blob in GREEN
        cv::circle(
            image, cv::Point( x, y ), radius, cv::Scalar( 0, 255, 0 ), 2 );
    }

    showMat( image, "Marked found coins", true );

    // Now e want to do a connected component analysis
    cv::Mat cvLabels;
    cv::Mat cvBlobStatistics;
    cv::Mat cvBlobCentroids;

    constexpr int lblType = CV_32S;

    const int32_t numBlobs = cv::connectedComponentsWithStats(
        dilated, cvLabels, cvBlobStatistics, cvBlobCentroids, 8, lblType );

    std::cout << "Number objects found with CCL: " << numBlobs << "\n";

    cv::Mat colorMap = displayConnectedComponents( cvLabels );

    showMat( colorMap, "Result of CCL", true );

    // Now we detect the coins used cv::findContours
    // We already know, that we only have to search for external contours. With
    // preprocessing we already closed each blob.

    // Find all contours in the image
    std::vector< std::vector< cv::Point > > contours;
    std::vector< cv::Vec4i > hierarchy;

    // Find external contours in the image
    cv::findContours( dilated,
                      contours,
                      hierarchy,
                      cv::RETR_EXTERNAL,
                      cv::CHAIN_APPROX_SIMPLE );

    std::cout << "Number contours found: " << contours.size( ) << "\n";

    std::vector< std::vector< cv::Point > > contoursList;
    std::vector< cv::Vec4i > hierarchyList;

    // Find external contours in the image
    cv::findContours( dilated,
                      contoursList,
                      hierarchyList,
                      cv::RETR_EXTERNAL,
                      cv::CHAIN_APPROX_SIMPLE );

    cv::Mat imageOuterContours = imageCopy.clone( );
    for ( size_t i = 0; i < contours.size( ); i++ )
    {
        auto& currentHierarchy = hierarchy[ i ];

        if ( currentHierarchy[ 3 ] == -1 )
        {
            cv::drawContours( imageOuterContours,
                              contours,
                              static_cast< int32_t >( i ),
                              cv::Scalar( 0, 255, 0 ),
                              3 );
        }
    }

    showMat( imageOuterContours, "Image with outer contours", true );

    // Draw all the contours
    cv::Mat imageContours = imageCopy.clone( );
    cv::Moments M;
    double area;
    double perimeter;
    std::vector< double > contourAreas;
    // contours.erase( contours.begin( ) + 1 );
    for ( size_t i = 0; i < contours.size( ); i++ )
    {
        // We will use the contour moments
        // to find the centroid
        M = moments( contours[ i ] );
        x = static_cast< int >( M.m10 / M.m00 );
        y = static_cast< int >( M.m01 / M.m00 );

        area = cv::contourArea( contours[ i ] );
        contourAreas.push_back( area );
        perimeter = cv::arcLength( contours[ i ], true );
        std::cout << "Contour #" << i + 1 << " has area = " << area
                  << " and perimeter = " << perimeter << '\n';

        // Mark the center
        cv::circle(
            imageContours, cv::Point( x, y ), 5, cv::Scalar( 255, 0, 0 ), -1 );

        cv::drawContours( imageContours,
                          contours,
                          static_cast< int32_t >( i ),
                          cv::Scalar( 0, 255, 0 ),
                          3 );
    }

    showMat( imageContours, "Image with contours", true );

    image = imageCopy.clone( );
    cv::Point2f center;
    float fradius;
    for ( size_t i = 0; i < contours.size( ); i++ )
    {
        // Fit a circle
        cv::minEnclosingCircle( contours[ i ], center, fradius );
        cv::circle( image,
                    center,
                    static_cast< int >( fradius ),
                    cv::Scalar( 255, 0, 0 ),
                    2 );
    }

    showMat( image, "Image with fittet circle", true );
}

void assignmentPartB( )
{
    constexpr float imageScaleFactor = 0.25f;

    // Image path
    std::string imagePath = IMAGES_ROOT + "/CoinsB.png";
    // Read image
    // Store it in the variable image
    ///
    /// YOUR CODE HERE
    ///
    // Read the image as color image
    cv::Mat image = cv::imread( imagePath, cv::IMREAD_COLOR );

    // Get a copy of the input image
    cv::Mat imageCopy = image.clone( );

    showMat( image, "Input image", false, imageScaleFactor );

    // Convert image to grayscale
    // Store it in the variable imageGray
    ///
    /// YOUR CODE HERE
    ///
    cv::Mat imageGray;
    cv::cvtColor( image, imageGray, cv::COLOR_BGR2GRAY );
    showMat( imageGray, "Input gray", false, imageScaleFactor );

    // Split cell into channels
    // Store them in variables imageB, imageG, imageR
    ///
    /// YOUR CODE HERE
    cv::Mat channels[ 3 ];
    cv::split( image, channels );

    // The channels are ordered BGR - no need to clone yet
    cv::Mat imageB = channels[ 0 ];
    cv::Mat imageG = channels[ 1 ];
    cv::Mat imageR = channels[ 2 ];

    showMat( imageB, "Channel Blue", false, imageScaleFactor );
    showMat( imageG, "Channel Green", false, imageScaleFactor );
    showMat( imageR, "Channel Red", false, imageScaleFactor );

    // The blue channel looks best for doing the segmentation.
    cv::Mat dst;
    cv::Mat imageToThresh = imageB.clone( );

    // Due to the structured background the image might have some noise after
    // segmentation. We perform a median filter to eliminate some noise. The
    // median is an edge preserving filter.
    cv::medianBlur( imageToThresh, imageToThresh, 5 );

    // We know, that the top left corner of the image contains only background
    // (in this case the bright background) information. Let's use a roi in that
    // region in that area to calculate mean and standard deviation of the gray
    // values in that region. The standard deviation gives us some information
    // about the scattering of the / values around the mean. This is interesting
    // because of the pattern in the / background. We'll use mean - 14 times the
    // deviation as threshold. The carpet on which the coins are placed is much
    // brighter than the coins, so we can subtract multiple times the deviation
    // from the carpet to segment teh coins. There is also a shading from top
    // left to bottom right visible. This also required multiple times the
    // deviation
    cv::Mat roi = imageToThresh( cv::Rect( 0, 0, 50, 50 ) );
    cv::Scalar mean;
    cv::Scalar stdDev;
    cv::meanStdDev( roi, mean, stdDev );
    double thresh = mean[ 0 ] - 14 * stdDev[ 0 ];
    std::cout << "Mean: " << mean[ 0 ] << "\n";
    std::cout << "StdDev: " << stdDev[ 0 ] << "\n";
    std::cout << "Threshold value to use: " << thresh << "\n";
    showMat( roi, "ROI", false );

    cv::threshold( imageToThresh, dst, thresh, 1, cv::THRESH_BINARY_INV | cv::THRESH_OTSU );
    showMat( dst * 255, "Segmented green channel", false, imageScaleFactor );

    // The coins seem to be segmented nicely. There is only some noise in the
    // image. We'll perform an erosion first. We do this with a circular shape
    // structure element. We choose a circular shape because the coins are
    // also circular. Using e.g. a rectangular will change the outer shape of
    // our coins. We also use a bigger kernel to make sure the artifact will
    // disappear. In the segmented image we can also see a kind of circular
    // shape at the center bottom position of the image. With a bigger erosion
    // kernel we can get rif of the as well.
    cv::Mat structureElement =
        cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 9, 9 ) );

    cv::Mat eroded;
    cv::morphologyEx( dst,
                      eroded,
                      cv::MORPH_ERODE,
                      structureElement,
                      cv::Point2i( -1, -1 ),
                      1 );

    showMat( eroded * 255, "Eroded objects", false, imageScaleFactor );

    // Since we reduced the size with the erosion, we have to dilate with the
    // same kernel to get the original object size back.

    cv::Mat dilated;
    cv::morphologyEx( eroded,
                      dilated,
                      cv::MORPH_DILATE,
                      structureElement,
                      cv::Point2i( -1, -1 ),
                      3 );

    // dilated *= 255;

    showMat( dilated * 255, "Dilated objects", false, imageScaleFactor );

    cv::Mat closed;
    cv::morphologyEx( dilated,
                      closed,
                      cv::MORPH_CLOSE,
                      structureElement,
                      cv::Point2i( -1, -1 ),
                      15 );

    showMat( closed * 255, "Closed objects", false, imageScaleFactor );

    closed *= 255;
    dilated *= 255;

    // Now it is time to detect the blobs. Since we are looking for coins we
    // can  expect circular shaped objects. We also know, that we are looking
    // for  bright objects, due to the decision for thresholding.
    // Setup cv::SimpleBlobDetector parameters.
    cv::SimpleBlobDetector::Params params;

    // Set color
    params.filterByColor = false;
    params.blobColor = 255;

    // Filter by Area
    params.filterByArea = false;

    // Filter by Circularity
    params.filterByCircularity = true;
    params.minCircularity = 0.75f;
    params.maxCircularity = 0.99f;

    // Filter by Convexity
    params.filterByConvexity = false;
    params.minConvexity = 0.8f;

    // Filter by Inertia
    params.filterByInertia = false;
    params.minInertiaRatio = 0.8f;

    // Set up detector with params
    cv::Ptr< cv::SimpleBlobDetector > detector =
        cv::SimpleBlobDetector::create( params );

    std::vector< cv::KeyPoint > keypoints;
    detector->detect( dilated, keypoints );

    std::cout << "Number blobs detected: " << keypoints.size( ) << "\n";

    int x, y;
    int radius;
    double diameter;

    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    cv::Scalar fontColor = cv::Scalar( 250, 0, 0 );
    int fontThickness = 4;
    int pixelHeight = 40;
    double fontScale =
        cv::getFontScaleFromHeight( fontFace, pixelHeight, fontThickness );

    // Lets display the detected blobs
    for ( int i = 0; i < static_cast< int >( keypoints.size( ) ); i++ )
    {
        cv::KeyPoint k = keypoints[ static_cast< size_t >( i ) ];
        cv::Point keyPt;
        keyPt = k.pt;
        x = keyPt.x;
        y = keyPt.y;
        // Mark center
        cv::circle( image, cv::Point( x, y ), 5, cv::Scalar( 255, 0, 0 ), -1 );
        // Get radius of coin
        diameter = k.size;
        radius = static_cast< int >( diameter / 2.0 );
        // Mark blob in GREEN
        cv::circle(
            image, cv::Point( x, y ), radius, cv::Scalar( 0, 255, 0 ), 2 );

        // Mark the contour number
        cv::putText( image,
                     std::to_string( i + 1 ),
                     cv::Point( x + 40, y - 10 ),
                     fontFace,
                     fontScale,
                     fontColor,
                     fontThickness );
    }

    showMat( image, "Marked found coins", true, imageScaleFactor );

    // Now e want to do a connected component analysis
    cv::Mat cvLabels;
    cv::Mat cvBlobStatistics;
    cv::Mat cvBlobCentroids;

    constexpr int lblType = CV_32S;

    const int32_t numBlobs = cv::connectedComponentsWithStats(
        dilated, cvLabels, cvBlobStatistics, cvBlobCentroids, 8, lblType );

    std::cout << "Number objects found with CCL: " << numBlobs << "\n";

    cv::Mat colorMap = displayConnectedComponents( cvLabels );

    showMat( colorMap, "Result of CCL", true, imageScaleFactor );

    // Now we detect the coins used cv::findContours
    // We already know, that we only have to search for external contours. With
    // preprocessing we already closed each blob.

    // Find all contours in the image
    std::vector< std::vector< cv::Point > > contours;
    std::vector< cv::Vec4i > hierarchy;

    // Find external contours in the image
    cv::findContours(
        dilated, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );

    std::cout << "Number contours found: " << contours.size( ) << "\n";

    cv::Mat imageContours = imageCopy.clone( );

    cv::drawContours( imageContours, contours, -1, cv::Scalar( 0, 255, 0 ), 3 );

    showMat( imageContours, "Image with all contours", true, imageScaleFactor );

    // Remove the inner contours
    auto cntBegin = contours.begin( );
    auto hirBegin = hierarchy.begin( );

    for ( int i = 0; cntBegin != contours.end( ); i++ )
    {
        if ( ( *hirBegin )[ 3 ] != -1 )
        {
            std::cout << "Removing inner contour at index: "
                      << std::to_string( i ) << "\n";
            cntBegin = contours.erase( cntBegin );
            hirBegin = hierarchy.erase( hirBegin );
        }
        else
        {
            cntBegin++;
            hirBegin++;
        }
    }

    // Draw all the contours
    imageContours = imageCopy.clone( );
    cv::Moments M;
    double area;
    double perimeter;
    std::vector< double > contourAreas;
    // contours.erase( contours.begin( ) + 1 );
    for ( size_t i = 0; i < contours.size( ); i++ )
    {
        // We will use the contour moments
        // to find the centroid
        M = moments( contours[ i ] );
        x = static_cast< int >( M.m10 / M.m00 );
        y = static_cast< int >( M.m01 / M.m00 );

        area = cv::contourArea( contours[ i ] );
        contourAreas.push_back( area );
        perimeter = cv::arcLength( contours[ i ], true );
        std::cout << "Contour #" << i + 1 << " has area = " << area
                  << " and perimeter = " << perimeter << '\n';

        // Mark the center
        cv::circle(
            imageContours, cv::Point( x, y ), 5, cv::Scalar( 255, 0, 0 ), -1 );

        cv::drawContours( imageContours,
                          contours,
                          static_cast< int32_t >( i ),
                          cv::Scalar( 0, 255, 0 ),
                          3 );
    }

    showMat( imageContours, "Image with contours", true, imageScaleFactor );

    image = imageCopy.clone( );
    cv::Point2f center;
    float fradius;
    for ( size_t i = 0; i < contours.size( ); i++ )
    {
        // Fit a circle
        cv::minEnclosingCircle( contours[ i ], center, fradius );
        cv::circle( image,
                    center,
                    static_cast< int >( fradius ),
                    cv::Scalar( 255, 0, 0 ),
                    2 );
    }

    showMat( image, "Image with fittet circle", true, imageScaleFactor );
}