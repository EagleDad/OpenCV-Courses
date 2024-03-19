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

cv::HOGDescriptor hog( cv::Size( 96, 32 ),       // winSize
                       cv::Size( 8, 8 ),         // blocksize
                       cv::Size( 8, 8 ),         // blockStride,
                       cv::Size( 4, 4 ),         // cellSize,
                       9,                        // nbins,
                       0,                        // derivAper,
                       4,                        // winSigma,
                       cv::HOGDescriptor::L2Hys, // histogramNormType,
                       0.2,                      // L2HysThresh,
                       1,                        // gammal correction,
                       64,                       // nlevels
                       1 );                      // signedGradient

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

void loadTrainTestLabel( std::string& pathName,
                         std::vector< cv::Mat >& trainImages,
                         std::vector< cv::Mat >& testImages,
                         std::vector< int >& trainLabels,
                         std::vector< int >& testLabels, int classVal,
                         float testFraction = 0.2 )
{
    const std::vector< std::string > imageFiles =
        getFileList( pathName, ".jpg" );

    const int totalImages = static_cast< int >( imageFiles.size( ) );
    const int nTest = static_cast< int >( testFraction *
                                          static_cast< float >( totalImages ) );

    for ( int counter = 0; counter < totalImages; counter++ )
    {
        cv::Mat image =
            cv::imread( imageFiles[ static_cast< size_t >( counter ) ] );
        if ( counter < nTest )
        {
            testImages.push_back( image );
            testLabels.push_back( classVal );
        }
        else
        {
            trainImages.push_back( image );
            trainLabels.push_back( classVal );
        }
    }
}

void getSVMParams( cv::Ptr< cv::ml::SVM > model )
{
    std::cout << "Kernel type     : " << model->getKernelType( ) << '\n';
    std::cout << "Type            : " << model->getType( ) << '\n';
    std::cout << "C               : " << model->getC( ) << '\n';
    std::cout << "Degree          : " << model->getDegree( ) << '\n';
    std::cout << "Nu              : " << model->getNu( ) << '\n';
    std::cout << "Gamma           : " << model->getGamma( ) << '\n';
}

void CreateHOG( std::vector< std::vector< float > >& HOG,
                std::vector< cv::Mat >& Images )
{
    for ( size_t y = 0; y < Images.size( ); y++ )
    {
        std::vector< float > descriptors;
        hog.compute( Images[ y ], descriptors );
        HOG.push_back( descriptors );
    }
}

void ConvertVectortoMatrix( std::vector< std::vector< float > >& HOG,
                            cv::Mat& samples )
{
    int descriptor_size = static_cast< int >( HOG[ 0 ].size( ) );

    for ( int i = 0; i < static_cast< int >( HOG.size( ) ); i++ )
        for ( int j = 0; j < descriptor_size; j++ )
            samples.at< float >( i, j ) =
                HOG[ static_cast< size_t >( i ) ][ static_cast< size_t >( j ) ];
}

auto svmInit( float C, float gamma )
{
    cv::Ptr< cv::ml::SVM > model = cv::ml::SVM::create( );
    model->setGamma( gamma );
    model->setC( C );
    model->setKernel( cv::ml::SVM::RBF );
    model->setType( cv::ml::SVM::C_SVC );

    return model;
}

void svmTrain( cv::Ptr< cv::ml::SVM > model, cv::Mat& samples,
               std::vector< int >& responses )
{
    cv::Ptr< cv::ml::TrainData > td =
        cv::ml::TrainData::create( samples, cv::ml::ROW_SAMPLE, responses );
    model->train( td );
    model->save( RESULTS_ROOT + "/eyeGlassClassifierModel.yml" );
}

void svmPredict( cv::Ptr< cv::ml::SVM > model, cv::Mat& testResponse,
                 cv::Mat& testMat )
{
    model->predict( testMat, testResponse );
}

void SVMevaluate( cv::Mat& testResponse, float& count, float& accuracy,
                  std::vector< int >& testLabels )
{
    for ( int i = 0; i < testResponse.rows; i++ )
    {
        // cout << testResponse.at<float>(i,0) << " " << testLabels[i] << endl;
        if ( testResponse.at< float >( i, 0 ) ==
             static_cast< float >( testLabels[ static_cast< size_t >( i ) ] ) )
            count = count + 1;
    }

    accuracy = ( count / static_cast< float >( testResponse.rows ) ) * 100.0f;
}

cv::Mat getCroppedEyeRegion( cv::Mat targetImage )
{
    cv::String faceCascadePath =
        IMAGES_ROOT + "/models/haarcascade_frontalface_default.xml";

    cv::CascadeClassifier faceCascade;
    if ( ! faceCascade.load( faceCascadePath ) )
    {
        printf( "--(!)Error loading face cascade\n" );
    };

    cv::Mat targetImageGray;

    cvtColor( targetImage, targetImageGray, cv::COLOR_BGR2GRAY );

    std::vector< cv::Rect > faces;
    faceCascade.detectMultiScale( targetImageGray, faces, 1.3, 5 );

    int x = faces[ 0 ].x;
    int y = faces[ 0 ].y;
    int w = faces[ 0 ].width;
    int h = faces[ 0 ].height;

    cv::Mat face_roi =
        targetImage( cv::Range( y, y + h ), cv::Range( x, x + w ) );
    imwrite( RESULTS_ROOT + "/face_roi.png", face_roi );
    int face_height, face_width;
    face_height = face_roi.size( ).height;
    face_width = face_roi.size( ).width;

    // Apply a heuristic formula for getting the eye region from face
    int eyeTop = static_cast< int >( 1.0 / 6.0 * face_height );
    int eyeBottom = static_cast< int >( 3.0 / 6.0 * face_height );

    std::cout << "Eye Height between : " << eyeTop << "," << eyeBottom << '\n';

    cv::Mat eye_roi =
        face_roi( cv::Range( eyeTop, eyeBottom ), cv::Range( 0, face_width ) );

    // Resize the eye region to a fixed size of 96x32
    cv::Mat cropped;
    resize( eye_roi, cropped, cv::Size( 96, 32 ) );

    return cropped;
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    std::vector< cv::Mat > trainImages;
    std::vector< cv::Mat > testImages;
    std::vector< int > trainLabels;
    std::vector< int > testLabels;

    // Path1 is class 0 and Path2 is class 1
    std::string path1 =
        IMAGES_ROOT + "/glassesDataset/cropped_withoutGlasses2/";
    std::string path2 = IMAGES_ROOT + "/glassesDataset/cropped_withGlasses2/";

    // Load Data into Train and Test variables
    // Get training and testing images for both classes
    loadTrainTestLabel(
        path1, trainImages, testImages, trainLabels, testLabels, 0 );
    loadTrainTestLabel(
        path2, trainImages, testImages, trainLabels, testLabels, 1 );

    // Compute Features
    std::vector< std::vector< float > > trainHOG;
    std::vector< std::vector< float > > testHOG;
    CreateHOG( trainHOG, trainImages );
    CreateHOG( testHOG, testImages );

    auto descriptor_size = static_cast< int >( trainHOG[ 0 ].size( ) );
    std::cout << "Descriptor Size : " << descriptor_size << '\n';

    // Convert hog data into features recognized by SVM model
    cv::Mat trainMat(
        static_cast< int >( trainHOG.size( ) ), descriptor_size, CV_32FC1 );
    cv::Mat testMat(
        static_cast< int >( testHOG.size( ) ), descriptor_size, CV_32FC1 );

    ConvertVectortoMatrix( trainHOG, trainMat );
    ConvertVectortoMatrix( testHOG, testMat );

    // Train the SVM Model
    float C = 2.5f, Gamma = 0.02f;

    cv::Mat testResponse;
    cv::Ptr< cv::ml::SVM > model = svmInit( C, Gamma );

    ///////////  SVM Training  ////////////////
    svmTrain( model, trainMat, trainLabels );

    // Evaluate the model and Check the Accuracy
    ///////////  SVM Testing  ////////////////
    svmPredict( model, testResponse, testMat );

    ////////////// Find Accuracy   ///////////
    float count = 0;
    float accuracy = 0;
    getSVMParams( model );
    SVMevaluate( testResponse, count, accuracy, testLabels );

    std::cout << "Percentage Accuracy : " << accuracy << '\n';

    // Check the result visually
    std::string filename = IMAGES_ROOT + "/glassesDataset/glasses_4.jpg";

    // Read the image
    cv::Mat testImage = cv::imread( filename );

    cv::Mat cropped = getCroppedEyeRegion( testImage );

    // Create vector of images for testing
    std::vector< cv::Mat > testImageArray;
    testImageArray.push_back( cropped );

    // Compute HOG descriptors
    std::vector< std::vector< float > > testHOGArray;
    CreateHOG( testHOGArray, testImageArray );

    // Convert the descriptors to Mat
    cv::Mat testSample(
        static_cast< int >( testHOGArray.size( ) ), descriptor_size, CV_32FC1 );
    ConvertVectortoMatrix( testHOGArray, testSample );

    // We will load the model again and test the model
    // This is just to explain how to load an SVM model
    // You can use the model directly too

    cv::Ptr< cv::ml::SVM > savedModel = cv::ml::StatModel::load< cv::ml::SVM >(
        RESULTS_ROOT + "/eyeGlassClassifierModel.yml" );
    cv::Mat pred;
    svmPredict( savedModel, pred, testSample );

    // 0 -> No Glasses
    // 1 -> With Glasses
    std::cout << "Prediction : " << pred.at< float >( 0, 0 ) << '\n';

    showMat( testImage, "Test Image", false );
    showMat( cropped, "Cropped Image", true );

    filename = IMAGES_ROOT + "/glassesDataset/no_glasses1.jpg";

    // Read the image
    testImage = cv::imread( filename );
    cropped = getCroppedEyeRegion( testImage );

    // Create vector of images for testing
    std::vector< cv::Mat > testImageArray2;
    testImageArray2.push_back( cropped );

    // Compute HOG descriptors
    std::vector< std::vector< float > > testHOGArray2;
    CreateHOG( testHOGArray2, testImageArray2 );

    // Convert the descriptors to Mat
    cv::Mat testSample2( static_cast<int>(testHOGArray2.size( )), descriptor_size, CV_32FC1 );
    ConvertVectortoMatrix( testHOGArray2, testSample2 );

    // We will load the model again and test the model
    // This is just to explain how to load an SVM model
    // You can use the model directly too
    cv::Mat pred2;
    svmPredict( savedModel, pred2, testSample2 );

    // 0 -> No Glasses
    // 1 -> With Glasses
    std::cout << "Prediction : " << pred2.at< float >( 0, 0 ) << '\n';

    showMat( testImage, "Test Image", false );
    showMat( cropped, "Cropped Image", true );

    // Clean up
    cv::destroyAllWindows( );

    return 0;
}