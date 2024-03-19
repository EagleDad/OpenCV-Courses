#include "Interface.h"

IGNORE_WARNINGS_OPENCV_PUSH
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
IGNORE_WARNINGS_POP

void Interface::mouseClick( int event, int x, int y, int flags, void* )
{
    // Set labels for events and then push labels(IN_PROCESS) to mask
    switch ( event )
    {
    case cv::EVENT_LBUTTONDOWN: // set rect or GC_BGD(GC_FGD) labels
    {
        if ( rectState == NOT_SET )
        {
            rectState = IN_PROCESS;
            rect = cv::Rect( x, y, 1, 1 );
        }
        if ( ( global_value == 1 || global_value == 2 ) &&
             rectState == SET ) // If it is sure area
            lblsState = IN_PROCESS;
        if ( ( global_value == 3 || global_value == 4 ) &&
             rectState == SET ) // Probable prFGD/prBGD
            prLblsState = IN_PROCESS;
    }
    break;

    case cv::EVENT_LBUTTONUP:
        if ( rectState == IN_PROCESS )
        {
            rect = cv::Rect( cv::Point( rect.x, rect.y ), cv::Point( x, y ) );
            rectState = SET;
            setRectInMask( );
            CV_Assert( BG_PIXEL.empty( ) && FG_PIXEL.empty( ) &&
                       prBG_PIXEL.empty( ) && prFG_PIXEL.empty( ) );
            showImage( );
        }
        if ( lblsState == IN_PROCESS )
        {
            setLblsInMask( flags, cv::Point( x, y ), false );
            lblsState = SET;
            showImage( );
        }
        if ( prLblsState == IN_PROCESS )
        {
            setLblsInMask( flags, cv::Point( x, y ), true );
            prLblsState = SET;
            showImage( );
        }
        break;

    case cv::EVENT_MOUSEMOVE:
        if ( rectState == IN_PROCESS )
        {
            rect = cv::Rect( cv::Point( rect.x, rect.y ), cv::Point( x, y ) );
            CV_Assert( BG_PIXEL.empty( ) && FG_PIXEL.empty( ) &&
                       prBG_PIXEL.empty( ) && prFG_PIXEL.empty( ) );
            showImage( );
        }
        else if ( lblsState == IN_PROCESS )
        {
            setLblsInMask( flags, cv::Point( x, y ), false );
            showImage( );
        }
        else if ( prLblsState == IN_PROCESS )
        {
            setLblsInMask( flags, cv::Point( x, y ), true );
            showImage( );
        }
        break;
    }
}

// Seting up Image
void Interface::setImageAndWinName( const cv::Mat& _image,
                                    const std::string& _winName )
{
    if ( _image.empty( ) || _winName.empty( ) )
        return;
    image = &_image;
    winName = &_winName;
    mask.create( image->size( ), CV_8UC1 );
    reset( );
}

void Interface::showImage( ) const
{
    if ( image->empty( ) || winName->empty( ) )
        return;

    cv::Mat res;
    cv::Mat binMask;
    if ( ! isInitialized )
        image->copyTo( res ); // Creating copy of Image to work with
    else
    {
        getBinMask( mask, binMask );
        image->copyTo( res, binMask );
    }

    // Selecting coloured Pixel for selected mouse events
    // e.g blue for background aarea selection
    std::vector< cv::Point >::const_iterator it;
    for ( it = BG_PIXEL.begin( ); it != BG_PIXEL.end( ); ++it )
        circle( res, *it, radius, BLUE, thickness );
    for ( it = FG_PIXEL.begin( ); it != FG_PIXEL.end( ); ++it )
        circle( res, *it, radius, RED, thickness );
    for ( it = prBG_PIXEL.begin( ); it != prBG_PIXEL.end( ); ++it )
        circle( res, *it, radius, LIGHTBLUE, thickness );
    for ( it = prFG_PIXEL.begin( ); it != prFG_PIXEL.end( ); ++it )
        circle( res, *it, radius, PINK, thickness );

    if ( rectState == IN_PROCESS || rectState == SET )
        rectangle( res,
                   cv::Point( rect.x, rect.y ),
                   cv::Point( rect.x + rect.width, rect.y + rect.height ),
                   GREEN,
                   2 );

    imshow( *winName, res );
}

void Interface::setRectInMask( )
{
    CV_Assert( ! mask.empty( ) );
    mask.setTo( cv::GC_BGD );
    rect.x = std::max( 0, rect.x );
    rect.y = std::max( 0, rect.y );
    rect.width = std::min( rect.width, image->cols - rect.x );
    rect.height = std::min( rect.height, image->rows - rect.y );
    ( mask( rect ) ).setTo( cv::Scalar( cv::GC_PR_FGD ) );
}

// Applying grabcut to selected areas by keys
void Interface::setLblsInMask( int flags, cv::Point p, bool isPr )
{
    std::vector< cv::Point >*bpxls, *fpxls;
    uchar bvalue, fvalue;
    if ( ! isPr )
    {
        bpxls = &BG_PIXEL;
        fpxls = &FG_PIXEL;
        bvalue = cv::GC_BGD;
        fvalue = cv::GC_FGD;
    }
    else
    {
        bpxls = &prBG_PIXEL;
        fpxls = &prFG_PIXEL;
        bvalue = cv::GC_PR_BGD;
        fvalue = cv::GC_PR_FGD;
    }

    // When area is sure background or foreground and we have to select
    // background i.e sure area is selected by variable lblsState and background
    // by value 1 and 3
    if ( flags & ( global_value == 1 || global_value == 3 ) )
    {
        bpxls->push_back( p );
        circle( mask, p, radius, bvalue, thickness );
    }
    if ( flags & ( global_value == 2 || global_value == 4 ) )
    {
        fpxls->push_back( p );
        circle( mask, p, radius, fvalue, thickness );
    }
}

void getBinMask( const cv::Mat& comMask, cv::Mat& binMask )
{
    if ( comMask.empty( ) || comMask.type( ) != CV_8UC1 )
        CV_Error( cv::Error::StsBadArg,
              "comMask is empty or has incorrect type (not CV_8UC1)" );
    if ( binMask.empty( ) || binMask.rows != comMask.rows ||
         binMask.cols != comMask.cols )
        binMask.create( comMask.size( ), CV_8UC1 );
    binMask = comMask & 1;
}

void on_mouse( int event, int x, int y, int flags, void* param )
{
    app.mouseClick( event, x, y, flags, param );
}

// Reset everything
void Interface::reset( )
{
    if ( ! mask.empty( ) )
        mask.setTo( cv::Scalar::all( cv::GC_BGD ) );
    BG_PIXEL.clear( );
    FG_PIXEL.clear( );
    prBG_PIXEL.clear( );
    prFG_PIXEL.clear( );

    isInitialized = false;
    rectState = NOT_SET;
    lblsState = NOT_SET;
    prLblsState = NOT_SET;
    iterCount = 0;
}

// Grabing image on each updation of foreground,background Labels ie (lblsState)
// or probable ...(prlblsState)
int Interface::nextIter( )
{
    if ( isInitialized )
        grabCut( *image, mask, rect, bgdModel, fgdModel, 1 );
    else
    {
        if ( rectState != SET ) // Rectangle State not defined
            return iterCount;

        if ( lblsState == SET || prLblsState == SET )
            grabCut( *image,
                     mask,
                     rect,
                     bgdModel,
                     fgdModel,
                     1,
                     cv::GC_INIT_WITH_MASK );
        else
            grabCut( *image,
                     mask,
                     rect,
                     bgdModel,
                     fgdModel,
                     1,
                     cv::GC_INIT_WITH_RECT );

        isInitialized = true;
    }
    iterCount++;

    BG_PIXEL.clear( );
    FG_PIXEL.clear( );
    prBG_PIXEL.clear( );
    prFG_PIXEL.clear( );

    return iterCount;
}