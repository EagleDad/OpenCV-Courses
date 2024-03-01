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
    // Path to the image we are going to read
    // This can be an absolute or relative path
    // Here we are using a relative path
    std::string imageName = IMAGES_ROOT + "/boy.jpg";

    // Load the image
    cv::Mat image;
    image = cv::imread( imageName, cv::IMREAD_COLOR );

    showMat( image, "Boy org", false );

    // Draw a line
    cv::Mat imageLine = image.clone( );

    // The line starts from (322,179) and ends at (400,183)
    // The color of the line is RED (Recall that OpenCV uses BGR format)
    // Thickness of line is 5px
    // Linetype is LINE_AA (anti aliased)

    cv::line( imageLine,
              cv::Point( 200, 80 ),
              cv::Point( 280, 80 ),
              cv::Scalar( 0, 255, 0 ),
              3,
              cv::LINE_AA );

    cv::imwrite( RESULTS_ROOT + "/line.jpg", imageLine );

    showMat( imageLine, "Boy with line", true );

    // Draw a circle
    cv::Mat imageCircle = image.clone( );
    // thickness -1 indicates filled circle
    cv::circle( imageCircle,
                cv::Point( 250, 125 ),
                100,
                cv::Scalar( 0, 0, 255 ),
                5,
                cv::LINE_AA );

    cv::imwrite( RESULTS_ROOT + "/circle.jpg", imageCircle );

    showMat( imageCircle, "Boy with circle", true );

    // Draw a filled circle
    cv::Mat imageFilledCircle = image.clone( );

    cv::circle( imageFilledCircle,
                cv::Point( 250, 125 ),
                100,
                cv::Scalar( 0, 0, 255 ),
                -1,
                cv::LINE_AA );

    cv::imwrite( RESULTS_ROOT + "/filledCircle.jpg", imageFilledCircle );

    showMat( imageFilledCircle, "Boy with filled circle", true );

    // Draw an ellipse
    // Note: Ellipse Centers and Axis lengths must be integers
    cv::Mat imageEllipse = image.clone( );

    ellipse( imageEllipse,
             cv::Point( 250, 125 ),
             cv::Point( 100, 50 ),
             0,
             0,
             360,
             cv::Scalar( 255, 0, 0 ),
             3,
             cv::LINE_AA );

    ellipse( imageEllipse,
             cv::Point( 250, 125 ),
             cv::Point( 100, 50 ),
             90,
             0,
             360,
             cv::Scalar( 0, 0, 255 ),
             3,
             cv::LINE_AA );

    cv::imwrite( RESULTS_ROOT + "/ellipse.jpg", imageEllipse );

    showMat( imageEllipse, "Boy with ellipse", true );

    // Draw an ellipse
    // Note: Ellipse Centers and Axis lengths must be integers
    cv::Mat imageEllipsePar = image.clone( );

    // Incomplete/Open ellipse
    ellipse( imageEllipsePar,
             cv::Point( 250, 125 ),
             cv::Point( 100, 50 ),
             0,
             180,
             360,
             cv::Scalar( 255, 0, 0 ),
             3,
             cv::LINE_AA );

    // Filled ellipse
    ellipse( imageEllipsePar,
             cv::Point( 250, 125 ),
             cv::Point( 100, 50 ),
             0,
             0,
             180,
             cv::Scalar( 0, 0, 255 ),
             -2,
             cv::LINE_AA );

    cv::imwrite( RESULTS_ROOT + "/ellipseFilled.jpg", imageEllipsePar );

    showMat( imageEllipsePar, "Boy with ellipse partial filled", true );

    // Draw a rectangle (thickness is a positive integer)
    cv::Mat imageRectangle = image.clone( );
    cv::rectangle( imageRectangle,
                   cv::Point( 170, 50 ),
                   cv::Point( 300, 200 ),
                   cv::Scalar( 255, 0, 255 ),
                   5,
                   cv::LINE_8 );

    cv::imwrite( RESULTS_ROOT + "/rectangle.jpg", imageRectangle );

    showMat( imageRectangle, "Boy with rectangle", true );

    // Put text on image
    cv::Mat imageText = image.clone( );
    std::string text = "I am studying";
    double fontScale = 1.5;
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    cv::Scalar fontColor = cv::Scalar( 250, 10, 10 );
    int fontThickness = 2;

    putText( imageText,
             text,
             cv::Point( 20, 350 ),
             fontFace,
             fontScale,
             fontColor,
             fontThickness,
             cv::LINE_AA );

    cv::imwrite( RESULTS_ROOT + "/text.jpg", imageText );

    showMat( imageText, "Boy with text", true );

    int pixelHeight = 20;

    // Calculate the fontScale
    fontScale =
        cv::getFontScaleFromHeight( fontFace, pixelHeight, fontThickness );
    std::cout << "fontScale = " << fontScale;

    cv::Mat imageTextFontScale;
    imageTextFontScale = image.clone( );
    putText( imageTextFontScale,
             "I am studying",
             cv::Point( 20, 350 ),
             fontFace,
             fontScale,
             fontColor,
             fontThickness,
             cv::LINE_AA );

    cv::imwrite( RESULTS_ROOT + "/text2.jpg", imageTextFontScale );

    showMat( imageTextFontScale, "Boy with text proper", true );

    cv::Mat imageGetTextSize;
    imageGetTextSize = image.clone( );
    int imageHeight = imageGetTextSize.rows;
    int imageWidth = imageGetTextSize.cols;

    // Get the text box height and width and also the baseLine
    int baseLine = 0;
    cv::Size textSize = cv::getTextSize(
        "I am studying", fontFace, fontScale, fontThickness, &baseLine );

    int textWidth = textSize.width;
    int textHeight = textSize.height;

    std::cout << "TextWidth = " << textWidth << ", TextHeight = " << textHeight
              << ", baseLine = " << baseLine << "\n";

    // Get the coordinates of text box bottom left corner
    // The xccordinate will be such that the text is centered
    int xcoordinate = ( imageWidth - textWidth ) / 2;
    // The y coordinate will be such that the entire box is just 10 pixels above
    // the bottom of image
    int ycoordinate = ( imageHeight - baseLine - 10 );

    std::cout << "TextBox Bottom Left = (" << xcoordinate << "," << ycoordinate
              << ")\n";

    // Draw the Canvas using a filled rectangle
    cv::Scalar canvasColor = cv::Scalar( 255, 255, 255 );
    cv::Point canvasBottomLeft( xcoordinate, ycoordinate + baseLine );
    cv::Point canvasTopRight( xcoordinate + textWidth,
                              ycoordinate - textHeight );

    cv::rectangle(
        imageGetTextSize, canvasBottomLeft, canvasTopRight, canvasColor, -1 );

    std::cout << "Canvas Bottom Left = " << canvasBottomLeft
              << ", Top Right = " << canvasTopRight << "\n";

    // Now draw the baseline ( just for reference )
    int lineThickness = 2;
    cv::Point lineLeft( xcoordinate, ycoordinate );
    cv::Point lineRight( xcoordinate + textWidth, ycoordinate );
    cv::Scalar lineColor = cv::Scalar( 0, 255, 0 );

    line( imageGetTextSize,
          lineLeft,
          lineRight,
          lineColor,
          lineThickness,
          cv::LINE_AA );

    // Finally Draw the text
    putText( imageGetTextSize,
             "I am studying",
             cv::Point( xcoordinate, ycoordinate ),
             fontFace,
             fontScale,
             fontColor,
             fontThickness,
             cv::LINE_AA );

    cv::imwrite( RESULTS_ROOT + "/text3.jpg", imageGetTextSize );

    showMat( imageGetTextSize, "Boy centered text with base line", true );

    return 0;
}