#include <GUI.h>
#include <macros.h>

// OpenCV includes
IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/ml.hpp>
#include <opencv2/opencv.hpp>
IGNORE_WARNINGS_POP

// STD includes
#include <filesystem>
#include <iostream>

const std::string IMAGES_ROOT = "C:/images";
const std::string RESULTS_ROOT = "C:/images/results";

//
// Data Handling utilities
//

std::vector< std::string > getFileList( const std::string& directory,
                                        const std::string& fileExtension )
{
    std::vector< std::string > fileList;
    for ( const auto& entry : std::filesystem::directory_iterator( directory ) )
    {
        if ( ! is_directory( entry ) )
        {
            if ( std::filesystem::path( entry ).extension( ) == fileExtension )
            {
                fileList.push_back( entry.path( ).string( ) );
            }
        }
    }
    return fileList;
}

// read images in a folder
// return vector of images and labels
void getDataset( std::string& pathName, int classVal,
                 std::vector< cv::Mat >& images, std::vector< int >& labels )
{
    std::vector< std::string > imageFiles = getFileList( pathName, ".jpg" );

    for ( size_t i = 0; i < imageFiles.size( ); i++ )
    {
        cv::Mat im = cv::imread( imageFiles[ i ] );
        images.push_back( im );
        labels.push_back( classVal );
    }
}

//
// Setup SVM Classifier
//

// Initialize SVM with parameters
auto svmInit( float C, float gamma )
{
    cv::Ptr< cv::ml::SVM > model = cv::ml::SVM::create( );
    model->setGamma( gamma );
    model->setC( C );
    model->setKernel( cv::ml::SVM::LINEAR );
    model->setType( cv::ml::SVM::C_SVC );
    return model;
}

// Train SVM on data and labels
void svmTrain( cv::Ptr< cv::ml::SVM > model, cv::Mat& samples,
               std::vector< int >& responses )
{
    cv::Ptr< cv::ml::TrainData > td =
        cv::ml::TrainData::create( samples, cv::ml::ROW_SAMPLE, responses );
    model->train( td );
}

// predict labels for given samples
void svmPredict( cv::Ptr< cv::ml::SVM > model, cv::Mat& testMat,
                 cv::Mat& testResponse )
{
    model->predict( testMat, testResponse );
}

// evaluate a model by comparing
// predicted labels and ground truth
void svmEvaluate( cv::Mat& testResponse, std::vector< int >& testLabels,
                  int& correct, float& error )
{
    for ( int i = 0; i < testResponse.rows; i++ )
    {
        if ( testResponse.at< float >( i, 0 ) ==
             static_cast< float >( testLabels[ static_cast< size_t >( i ) ] ) )
            correct = correct + 1;
    }

    error = static_cast< float >( testLabels.size( ) - correct ) * 100.0f /
            static_cast< float >( testLabels.size( ) );
}

//
// Setup HoG Feature Detector
//

// Initialize HOG
cv::HOGDescriptor hog( cv::Size( 64, 128 ),      // winSize
                       cv::Size( 16, 16 ),       // blocksize
                       cv::Size( 8, 8 ),         // blockStride,
                       cv::Size( 8, 8 ),         // cellSize,
                       9,                        // nbins,
                       0,                        // derivAperture,
                       -1,                       // winSigma,
                       cv::HOGDescriptor::L2Hys, // histogramNormType,
                       0.2,                      // L2HysThresh,
                       true,                     // gammal correction,
                       64,                       // nlevels=64
                       false );                  // signedGradient

// OpenCV's HOG based Pedestrian Detector
cv::HOGDescriptor hogDefault( cv::Size( 64, 128 ),      // winSize
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

// compute HOG features for given images
void computeHOG( std::vector< std::vector< float > >& hogFeatures,
                 std::vector< cv::Mat >& images )
{
    for ( size_t y = 0; y < images.size( ); y++ )
    {
        std::vector< float > descriptor;
        hog.compute( images[ y ], descriptor );
        hogFeatures.push_back( descriptor );
    }
}

// Convert HOG descriptors to format recognized by SVM
void prepareData( std::vector< std::vector< float > >& hogFeatures,
                  cv::Mat& data )
{
    int descriptorSize = static_cast< int >( hogFeatures[ 0 ].size( ) );

    for ( int i = 0; i < static_cast< int >( hogFeatures.size( ) ); i++ )
        for ( int j = 0; j < descriptorSize; j++ )
            data.at< float >( i, j ) =
                hogFeatures[ static_cast< size_t >( i ) ]
                           [ static_cast< size_t >( j ) ];
}

//
// Setup Training and Testing Modes
//
// Flags to turn on/off training or testing
bool trainModel = true;
bool testModel = true;

// Path to INRIA Person dataset
std::string rootDir = IMAGES_ROOT + "/INRIAPerson/";

// set Train and Test directory paths
std::string trainDir = rootDir + "train_64x128_H96/";
std::string testDir = rootDir + "test_64x128_H96/";

//
// Training
//

//
// Main program
//

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    // ================================ Train Model
    // =============================================
    if ( trainModel == 1 )
    {
        std::string trainPosDir = trainDir + "posPatches/";
        std::string trainNegDir = trainDir + "negPatches/";

        std::vector< cv::Mat > trainPosImages, trainNegImages;
        std::vector< int > trainPosLabels, trainNegLabels;

        // Label 1 for positive images and -1 for negative images
        getDataset( trainPosDir, 1, trainPosImages, trainPosLabels );
        getDataset( trainNegDir, -1, trainNegImages, trainNegLabels );

        // Print total number of positive and negative examples
        std::cout << "positive - " << trainPosImages.size( ) << " , "
                  << trainPosLabels.size( ) << '\n';
        std::cout << "negative - " << trainNegImages.size( ) << " , "
                  << trainNegLabels.size( ) << '\n';

        // Append Positive/Negative Images/Labels for Training
        std::vector< cv::Mat > trainImages;
        std::vector< int > trainLabels;
        trainImages = trainPosImages;
        trainImages.insert( trainImages.end( ),
                            trainNegImages.begin( ),
                            trainNegImages.end( ) );

        trainLabels = trainPosLabels;
        trainLabels.insert( trainLabels.end( ),
                            trainNegLabels.begin( ),
                            trainNegLabels.end( ) );

        // Compute HOG features for images
        std::vector< std::vector< float > > hogTrain;
        computeHOG( hogTrain, trainImages );

        // Convert hog features into data format recognized by SVM
        int descriptorSize = static_cast< int >( hogTrain[ 0 ].size( ) );
        std::cout << "Descriptor Size : " << descriptorSize << '\n';
        cv::Mat trainData(
            static_cast< int >( hogTrain.size( ) ), descriptorSize, CV_32FC1 );
        prepareData( hogTrain, trainData );

        // Initialize SVM object
        float C = 0.01f, gamma = 0.0f;
        cv::Ptr< cv::ml::SVM > svm = svmInit( C, gamma );
        svmTrain( svm, trainData, trainLabels );
        svm->save( RESULTS_ROOT + "/pedestrian.yml" );
    }

    // ================================ Test Model
    // =============================================
    if ( testModel == 1 )
    {
        // Load model from saved file
        cv::Ptr< cv::ml::SVM > svm =
            cv::ml::SVM::load( RESULTS_ROOT + "/pedestrian.yml" );

        // We will test our model on positive and negative images separately
        // Read images from Pos and Neg directories
        std::string testPosDir = testDir + "posPatches/";
        std::string testNegDir = testDir + "negPatches/";

        std::vector< cv::Mat > testPosImages, testNegImages;
        std::vector< int > testPosLabels, testNegLabels;

        // Label 1 for positive images and -1 for negative images
        getDataset( testPosDir, 1, testPosImages, testPosLabels );
        getDataset( testNegDir, -1, testNegImages, testNegLabels );

        // Print total number of positive and negative examples
        std::cout << "positive - " << testPosImages.size( ) << " , "
                  << testPosLabels.size( ) << '\n';
        std::cout << "negative - " << testNegImages.size( ) << " , "
                  << testNegLabels.size( ) << '\n';

        // =========== Test on Positive Images ===============
        // Compute HOG features for images
        std::vector< std::vector< float > > hogPosTest;
        computeHOG( hogPosTest, testPosImages );

        // Convert hog features into data format recognized by SVM
        int descriptorSize = static_cast< int >( hogPosTest[ 0 ].size( ) );
        std::cout << "Descriptor Size : " << descriptorSize << '\n';
        cv::Mat testPosData( static_cast< int >( hogPosTest.size( ) ),
                             descriptorSize,
                             CV_32FC1 );
        prepareData( hogPosTest, testPosData );
        std::cout << testPosData.rows << " " << testPosData.cols << '\n';

        // Run classification on test images
        cv::Mat testPosPredict;
        svmPredict( svm, testPosData, testPosPredict );
        int posCorrect = 0;
        float posError = 0;
        svmEvaluate( testPosPredict, testPosLabels, posCorrect, posError );

        // Calculate True and False Positives
        int tp = posCorrect;
        int fp = static_cast< int >( testPosLabels.size( ) ) - posCorrect;
        std::cout << "TP: " << tp << " FP: " << fp
                  << " total: " << testPosLabels.size( )
                  << " error: " << posError << '\n';

        // =========== Test on Negative Images ===============
        // Compute HOG features for images
        std::vector< std::vector< float > > hogNegTest;
        computeHOG( hogNegTest, testNegImages );

        // Convert hog features into data format recognized by SVM
        std::cout << "Descriptor Size : " << descriptorSize << '\n';
        cv::Mat testNegData( static_cast< int >( hogNegTest.size( ) ),
                             descriptorSize,
                             CV_32FC1 );
        prepareData( hogNegTest, testNegData );

        // Run classification on test images
        cv::Mat testNegPredict;
        svmPredict( svm, testNegData, testNegPredict );
        int negCorrect = 0;
        float negError = 0;
        svmEvaluate( testNegPredict, testNegLabels, negCorrect, negError );

        // Calculate True and False Negatives
        int tn = negCorrect;
        int fn = static_cast< int >( testNegLabels.size( ) ) - negCorrect;
        std::cout << "TN: " << tn << " FN: " << fn
                  << " total: " << testNegLabels.size( )
                  << " error: " << negError << '\n';

        // Calculate Precision and Recall
        float precision = static_cast< float >( tp ) * 100.0f /
                          static_cast< float >( tp + fp );
        float recall = static_cast< float >( tp ) * 100.0f /
                       static_cast< float >( tp + fn );
        std::cout << "Precision: " << precision << " Recall: " << recall
                  << '\n';
    }

    // Comparison of our trained model with OpenCV's Pedestrian Detector

    cv::Ptr< cv::ml::SVM > svm =
        cv::ml::SVM::load( RESULTS_ROOT + "/pedestrian.yml" );
    // get the support vectors
    cv::Mat sv = svm->getSupportVectors( );
    // get the decision function
    cv::Mat alpha, svidx;
    double rho = svm->getDecisionFunction( 0, alpha, svidx );

    std::vector< float > svmDetectorTrained;
    svmDetectorTrained.clear( );
    svmDetectorTrained.resize( static_cast< size_t >( sv.cols ) + 1 );
    for ( int j = 0; j < sv.cols; j++ )
    {
        svmDetectorTrained[ static_cast< size_t >( j ) ] =
            -sv.at< float >( 0, j );
    }
    svmDetectorTrained[ static_cast< size_t >( sv.cols ) ] =
        static_cast< float >( rho );

    // set SVMDetector trained by us in HOG
    hog.setSVMDetector( svmDetectorTrained );

    // Set the people detector.
    std::vector< float > svmDetectorDefault = hog.getDefaultPeopleDetector( );
    hogDefault.setSVMDetector( svmDetectorDefault );

    std::string imageDir = IMAGES_ROOT + "/pedestrians";
    std::vector< std::string > imagePaths = getFileList( imageDir, ".jpg" );

    cv::Mat img = cv::imread( imagePaths[ 0 ], cv::IMREAD_COLOR );
    showMat( img, "Pedestrians", true, 0.5 );

    float finalHeight = 800.0, finalWidth;
    for ( size_t i = 0; i < imagePaths.size( ); i++ )
    {
        std::cout << "processing: " << imagePaths[ i ] << '\n';
        img = cv::imread( imagePaths[ i ], cv::IMREAD_COLOR );

        finalWidth = ( finalHeight * static_cast< float >( img.cols ) ) /
                     static_cast< float >( img.rows );
        cv::resize( img,
                    img,
                    cv::Size( static_cast< int >( finalWidth ),
                              static_cast< int >( finalHeight ) ) );

        // Detect people using trained and default SVM detectors
        std::vector< cv::Rect > bboxes, bboxes2;
        std::vector< double > weights, weights2;

        float hitThreshold = 1.0;
        cv::Size winStride = cv::Size( 8, 8 );
        cv::Size padding = cv::Size( 32, 32 );
        float scale = 1.05f;
        float finalThreshold = 2;
        bool useMeanshiftGrouping = 0;
        hog.detectMultiScale( img,
                              bboxes,
                              weights,
                              hitThreshold,
                              winStride,
                              padding,
                              scale,
                              finalThreshold,
                              useMeanshiftGrouping );

        hogDefault.detectMultiScale( img,
                                     bboxes2,
                                     weights2,
                                     0,
                                     winStride,
                                     padding,
                                     scale,
                                     finalThreshold,
                                     useMeanshiftGrouping );

        // Draw found rectangles on image. We will draw
        // green boxes for people detected by trained
        // model and red boxes for people detected by
        // OpenCV’s default model.
        if ( ! bboxes.empty( ) )
        {
            std::cout << "Trained Detector :: pedestrians detected: "
                      << bboxes.size( ) << '\n';
            std::vector< cv::Rect >::const_iterator loc = bboxes.begin( );
            std::vector< cv::Rect >::const_iterator end = bboxes.end( );
            for ( ; loc != end; ++loc )
            {
                cv::rectangle( img, *loc, cv::Scalar( 0, 255, 0 ), 2 );
            }
        }
        if ( ! bboxes2.empty( ) )
        {
            std::cout << "Default Detector :: pedestrians detected: "
                      << bboxes2.size( ) << '\n';
            std::vector< cv::Rect >::const_iterator loc = bboxes2.begin( );
            std::vector< cv::Rect >::const_iterator end = bboxes2.end( );
            for ( ; loc != end; ++loc )
            {
                cv::rectangle( img, *loc, cv::Scalar( 0, 0, 255 ), 2 );
            }
        }
        // Finally show the result and also write it to disk
        cv::imwrite(
            RESULTS_ROOT + "/results/Pedestrian_" + std::to_string( i ) + ".jpg", img );

        showMat( img, "Result " + std::to_string( i ), true , 0.5);
    }

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}