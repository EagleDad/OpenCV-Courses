#include <GUI.h>
#include <macros.h>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/parallel.hpp>
#include <fstream>

// Global constants
const std::string IMAGES_ROOT = "C:/images";
const std::string MODELS_ROOT = "C:/images/models";
const std::string RESULTS_ROOT = "C:/images/results";

constexpr size_t inWidth = 300;
constexpr size_t inHeight = 300;
constexpr double inScaleFactor = 1.0;
constexpr float confidenceThreshold = 0.6f;
const cv::Scalar meanVal(104.0, 177.0, 123.0);

const std::string caffeConfigFile = MODELS_ROOT + "/deploy.prototxt";
const std::string caffeWeightFile = MODELS_ROOT + "/res10_300x300_ssd_iter_140000_fp16.caffemodel";
const std::string tensorflowConfigFile = MODELS_ROOT + "/opencv_face_detector.pbtxt";
const std::string tensorflowWeightFile = MODELS_ROOT + "/opencv_face_detector_uint8.pb";

class ImageProcessor
{
public:
    ImageProcessor()
        : fontFace(cv::FONT_HERSHEY_SIMPLEX),
          fontColor(cv::Scalar(0, 255, 0)),
          fontThickness(2),
          blurRadius(1),
          blurRadiusMax(15),
          skinDetector(1),
          skinDetectorMax(1),
          textPixelHeight(20),
          windowName("Instagram Filters")
    {
#ifdef CAFFE
        net = cv::dnn::readNetFromCaffe(caffeConfigFile, caffeWeightFile);
#else
        net = cv::dnn::readNetFromTensorflow(tensorflowWeightFile, tensorflowConfigFile);
#endif
    }

    bool loadImage(const std::string &imagePath)
    {
        sourceImage = cv::imread(imagePath);
        if (sourceImage.empty())
        {
            std::cerr << "Error: Unable to load image!" << std::endl;
            return false;
        }

        // Resize image to fit within a normal screen
        if (sourceImage.cols > 512)
        {
            const auto scale = 512.0 / sourceImage.cols;
            cv::resize(sourceImage, sourceImage, cv::Size(), scale, scale);
        }

        resultImage = sourceImage.clone();
        overlayImage = sourceImage.clone();
        return true;
    }

    void process()
    {
        setupUI();
        updateView(resultImage);

        int key = 0;
        while (key != 27) // ESC to exit
        {
            if (key == 's') // Skin smoothing
            {
                reset();
                skinSmoothing();
            }
            else if (key == 'r') // Reset
            {
                reset();
            }
            key = cv::waitKey(20) & 0xFF;
        }

        cv::destroyAllWindows();
    }

private:
    cv::dnn::Net net;
    cv::Mat sourceImage, resultImage, overlayImage, combinedImage, helpImage, headerImage;
    std::string windowName;
    int blurRadius, blurRadiusMax, skinDetector, skinDetectorMax;
    int fontFace, fontThickness, textPixelHeight;
    cv::Scalar fontColor;

    void setupUI()
    {
        cv::namedWindow(windowName);
        cv::createTrackbar("Blur", windowName, &blurRadius, blurRadiusMax, applySkinSmoothing, this);
        cv::createTrackbar("Skin", windowName, &skinDetector, skinDetectorMax, applySkinSmoothing, this);
    }

    void reset()
    {
        resultImage = sourceImage.clone();
        overlayImage = resultImage.clone();
        updateView(resultImage);
    }

    static void applySkinSmoothing(int, void *userdata)
    {
        static_cast<ImageProcessor *>(userdata)->skinSmoothing();
    }

    void skinSmoothing()
    {
        reset();

        // Detect faces in parallel
        auto faceLocations = detectFaces(sourceImage);

        // Apply skin smoothing in parallel
        cv::parallel_for_(cv::Range(0, static_cast<int>(faceLocations.size())), [&](const cv::Range &range) {
            for (int i = range.start; i < range.end; ++i)
            {
                const auto &loc = faceLocations[i];
                processFaceRegion(loc);
            }
        });

        updateView(resultImage);
    }

    std::vector<cv::Rect> detectFaces(const cv::Mat &image)
    {
        std::vector<cv::Rect> faceLocations;

        const auto imageHeight = image.rows;
        const auto imageWidth = image.cols;

        const cv::Mat inputBlob = cv::dnn::blobFromImage(image, inScaleFactor, cv::Size(inWidth, inHeight), meanVal, false, false);
        net.setInput(inputBlob, "data");

        const cv::Mat detection = net.forward("detection_out");
        const cv::Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());

        for (int i = 0; i < detectionMat.rows; ++i)
        {
            const float confidence = detectionMat.at<float>(i, 2);
            if (confidence > confidenceThreshold)
            {
                const int x1 = static_cast<int>(detectionMat.at<float>(i, 3) * imageWidth);
                const int y1 = static_cast<int>(detectionMat.at<float>(i, 4) * imageHeight);
                const int x2 = static_cast<int>(detectionMat.at<float>(i, 5) * imageWidth);
                const int y2 = static_cast<int>(detectionMat.at<float>(i, 6) * imageHeight);

                if (x2 < imageWidth && y2 < imageHeight && x1 >= 0 && y1 >= 0)
                {
                    faceLocations.emplace_back(cv::Point(x1, y1), cv::Point(x2, y2));
                }
            }
        }

        return faceLocations;
    }

    void processFaceRegion(const cv::Rect &faceRegion)
    {
        auto mask = calculateMask(faceRegion);

        cv::Mat roi = sourceImage(faceRegion).clone();
        cv::Mat roiF;
        roi.convertTo(roiF, CV_32F, 1.0 / 255.0);

        mask.convertTo(mask, CV_32F, 1 / 255.0f);
        cv::GaussianBlur(mask, mask, cv::Size(7, 7), 0, 0);

        cv::Mat blurred;
        cv::GaussianBlur(roiF, blurred, cv::Size(blurRadius * 2 + 1, blurRadius * 2 + 1), 0);

        cv::Mat mask3F;
        cv::merge(std::vector<cv::Mat>{mask, mask, mask}, mask3F);

        cv::multiply(blurred, mask3F, blurred);
        mask3F.convertTo(mask3F, -1, -1, 1);
        cv::multiply(roiF, mask3F, roiF);

        blurred += roiF;
        blurred.convertTo(roi, CV_8U, 255.0f);

        roi.copyTo(resultImage(faceRegion));
    }

    cv::Mat calculateMask(const cv::Rect &faceLocation)
    {
        switch (skinDetector)
        {
        case 0:
            return meanColorSkinMask(faceLocation);
        case 1:
            return zeroSumGameTheoryModelSkinMask(faceLocation);
        default:
            return cv::Mat();
        }
    }

    cv::Mat zeroSumGameTheoryModelSkinMask(const cv::Rect &faceLocation)
    {
        cv::Mat roi = sourceImage(faceLocation);
        cv::Mat imageHsv, imageYCrCb;

        cv::cvtColor(roi, imageHsv, cv::COLOR_BGR2HSV);
        cv::cvtColor(roi, imageYCrCb, cv::COLOR_BGR2YCrCb);

        cv::Mat maskHsv, maskYCrCb;
        cv::inRange(imageHsv, cv::Scalar(0, 15, 0), cv::Scalar(17, 170, 255), maskHsv);
        cv::inRange(imageYCrCb, cv::Scalar(0, 135, 85), cv::Scalar(255, 180, 135), maskYCrCb);

        const cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
        cv::morphologyEx(maskHsv, maskHsv, cv::MORPH_OPEN, element);
        cv::morphologyEx(maskYCrCb, maskYCrCb, cv::MORPH_OPEN, element);

        cv::Mat finalMask;
        cv::bitwise_and(maskHsv, maskYCrCb, finalMask);
        cv::morphologyEx(finalMask, finalMask, cv::MORPH_OPEN, element);

        return finalMask;
    }

    cv::Mat meanColorSkinMask(const cv::Rect &faceLocation)
    {
        cv::Mat roi = sourceImage(faceLocation);
        cv::Mat imageHsv;
        cv::cvtColor(roi, imageHsv, cv::COLOR_BGR2HSV);

        cv::Scalar mean, stdDev;
        cv::meanStdDev(imageHsv, mean, stdDev);

        cv::Scalar lowerBound(mean[0] - stdDev[0], mean[1] - stdDev[1], 0);
        cv::Scalar upperBound(mean[0] + stdDev[0], mean[1] + stdDev[1], 255);

        cv::Mat mask;
        cv::inRange(imageHsv, lowerBound, upperBound, mask);

        constexpr double maxHue = 360;

        if (lowerBound[0] < 0 && upperBound[0] < maxHue)
        {
            cv::Mat tmpMask;
            lowerBound[0] += maxHue;
            upperBound[0] = maxHue;
            cv::inRange(imageHsv, lowerBound, upperBound, tmpMask);
            cv::bitwise_or(mask, tmpMask, mask);
        }
        else if (lowerBound[0] > 0 && upperBound[0] > maxHue)
        {
            cv::Mat tmpMask;
            lowerBound[0] = 0;
            upperBound[0] -= maxHue;
            cv::inRange(imageHsv, lowerBound, upperBound, tmpMask);
            cv::bitwise_or(mask, tmpMask, mask);
        }

        return mask;
    }

    void updateView(const cv::Mat &result)
    {
        cv::imshow(windowName, result);
    }
};

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return -1;
    }

    ImageProcessor processor;
    if (!processor.loadImage(argv[1]))
        return -1;

    processor.process();
    return 0;
}
