#include "DETECTION.h"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <string>

using namespace cv;
using namespace std;

#define CROP_CEILING 0.25  // 0 to remove crop ceiling
#define PROXIMITY_TH 80    // Min distance to trigger object detected "cm" 80 will be good

#define FOCAL_LENGTH 123
#define STEREO_BASELINE 321
#define DISPARITY_CONSTANT 3330  // Take this number divide by grayscale value to get "cm" in real world





DETECTION::DETECTION(Mat image)
{
    this->detectImage = image;

    // Image Size calculation
    this->imageHeight = this->detectImage.size().height;
    this->imageWdith = this->detectImage.size().width;
    int divisionLR = this->imageWdith/3; 
    int divisionMiddle = this->imageWdith - (2*divisionLR);
    int ceiling = CROP_CEILING*this->imageHeight;
    int rectHeight = this->imageHeight - ceiling;
    // printf("Image height: %d\n", this->imageHeight);
    // printf("Image width: %d\n", this->imageWdith);
    // printf("divisionLR: %d\n", divisionLR);
    // printf("divisionMiddle: %d\n", divisionMiddle);
    
    // Crop Left, Middle, Right 
    cv::Rect rectLeft(0, ceiling, divisionLR, rectHeight);
    cv::Rect rectMiddle(divisionLR, ceiling, divisionMiddle, rectHeight);
    cv::Rect rectRight(divisionLR+divisionMiddle, ceiling, divisionLR, rectHeight);
    cv::Rect rectHand(700, 350, 100, 100);

    // Mat For each division
    Mat roiLeft = this->detectImage(rectLeft);
    Mat roiMiddle = this->detectImage(rectMiddle);
    Mat roiRight = this->detectImage(rectRight);
    Mat roiHand = this->detectImage(rectHand);

    // Get Mean (MEAM lol)
    cv::Scalar meamLeft = mean(roiLeft);
    cv::Scalar meamMiddle = mean(roiMiddle);
    cv::Scalar meamRight = mean(roiRight);
    cv::Scalar meamHand = mean(roiHand);
    this->meanLeft = meamLeft[0];
    this->meanMiddle = meamMiddle[0];
    this->meanRight = meamRight[0];
    // std::cout<<"Mean Left: "<< this->meanLeft <<std::endl;
    // std::cout<<"Mean Middle: "<< this->meanMiddle <<std::endl;
    // std::cout<<"Mean Right: "<< this->meanRight <<std::endl;
    // std::cout<<"Mean Hand: "<< meamHand[0] <<std::endl;

    // Get Max Min Value in each division
    // double minLeft, maxLeft;
    cv::minMaxLoc(roiLeft, &this->minLeft, &this->maxLeft);
    // std::cout<<"Min Left: "<< this->minLeft << " Max Left:" << this->maxLeft <<std::endl;
    // double minMiddle, maxMiddle;
    cv::minMaxLoc(roiMiddle, &this->minMiddle, &this->maxMiddle);
    // std::cout<<"Min Middle: "<< this->minMiddle << " Max Middle:" << this->maxMiddle <<std::endl;
    // double minRight, maxRight;
    cv::minMaxLoc(roiLeft, &this->minRight, &this->maxRight);
    // std::cout<<"Min Right: "<< this->minRight << " Max Right:" << this->maxRight <<std::endl;
    // double minHand, maxHand;
    // cv::minMaxLoc(roiHand, &minHand, &maxHand);
    // std::cout<<"Min Hand: "<< minHand << " Max Hand:" << maxHand <<std::endl;


    // get standard deviation in each division
    cv::meanStdDev( roiLeft, this->dummyVec, this->stdDevLeft);
    // std::cout<<"StdDev Left: " << this->stdDevLeft[0] << std::endl;
    cv::meanStdDev( roiMiddle, this->dummyVec, this->stdDevMiddle);
    // std::cout<<"StdDev Middle: " << this->stdDevMiddle[0] << std::endl;
    cv::meanStdDev( roiRight, this->dummyVec, this->stdDevRight);
    // std::cout<<"StdDev Right: " << this->stdDevRight[0] << std::endl;

    
    // Draw 3 boundaries on image
    cv::rectangle(this->detectImage, rectLeft, cv::Scalar(0, 255, 0), 2);
    cv::rectangle(this->detectImage, rectMiddle, cv::Scalar(255, 0, 0), 2);
    cv::rectangle(this->detectImage, rectRight, cv::Scalar(0, 0, 255), 2);
    // cv::rectangle(this->detectImage, rectHand, cv::Scalar(0, 255, 0), 2);


    getDistance();

    // std::cout<<"Left:   " << "avgDist = " << this->avgDistLeft << " minDist = " << this->minDistLeft << " stdDev = " << this->stdDevLeft[0] << std::endl;
    // std::cout<<"Middle: " << "avgDist = " << this->avgDistMiddle << " minDist = " << this->minDistMiddle << " stdDev = " << this->stdDevMiddle[0] << std::endl;
    // std::cout<<"Right:  " << "avgDist = " << this->avgDistRight << " minDist = " << this->minDistRight << " stdDev = " << this->stdDevRight[0] << std::endl;

    FLC flcLeft( "left", this->avgDistLeft, this->minDistLeft, this->stdDevLeft[0]);
    FLC flcMiddle( "middle", this->avgDistMiddle, this->minDistMiddle, this->stdDevMiddle[0]);
    FLC flcRight( "right", this->avgDistRight, this->minDistRight, this->stdDevRight[0]);

    this->objLeft = flcLeft.compute();
    this->objMiddle = flcMiddle.compute();
    this->objRight = flcRight.compute();

#if 0 /* TBD: needs testing */
    clientsocket client("172.20.10.7", 9874);
    // uint32_t msg = 0x00;

    // msg |= (0x00 << 16);
    // msg |= (0x02 << 8);
    // msg |= (0x02);

    char msg[3];
    
    // left frame
    if ( this->objLeft == FO_CAUTION || this->objLeft == FO_CLEAR)
        msg[0] = 0;
    else
        msg[0] = 2;

    // middle frame
    if ( this->objMiddle == FO_CAUTION || this->objMiddle == FO_CLEAR)
        msg[1] = 0;
    else
        msg[1] = 2;

    // right frame
    if ( this->objRight == FO_CAUTION || this->objRight == FO_CLEAR)
        msg[2] = 0;
    else
        msg[2] = 2;


    client.send( msg);

#endif

    //thresholdDetection();

    //motorControl();

    shadeImage(roiLeft, roiMiddle, roiRight);

    //! [window]
    namedWindow( "Display window", WINDOW_AUTOSIZE ); // Create a window for display.
    // //! [window]

    // //! [imshow]
    imshow( "Display window", this->detectImage );                // Show our image inside it.
    //! [imshow]

    //! [wait]
    // waitKey(0); // Wait for a keystroke in the window
    //! [wait]
}

DETECTION:: ~DETECTION()
{

}

void DETECTION:: getDistance() 
{
    // Left 
    this->avgDistLeft = DISPARITY_CONSTANT / this->meanLeft;
    this->avgDistMiddle = DISPARITY_CONSTANT / this->meanMiddle;
    this->avgDistRight = DISPARITY_CONSTANT / this->meanRight;
    // std::cout<<"Avg Distance Left: "<< this->avgDistLeft <<std::endl;
    // std::cout<<"Avg Distance Middle: "<< this->avgDistMiddle <<std::endl;
    // std::cout<<"Avg Distance Right: "<< this->avgDistRight <<std::endl;

    /* TBD: don't know why this doesn't work well when
            DISPARITY_CONSTANT is divided by this->min<POSITION> */
    this->minDistLeft = this->minLeft;
    this->minDistMiddle = this->minMiddle;
    this->minDistRight = this->minRight;
}

/*
void DETECTION:: thresholdDetection() 
{
    this->objLeft = (this->avgDistLeft < PROXIMITY_TH) ? true : false;
    this->objMiddle = (this->avgDistMiddle < PROXIMITY_TH) ? true : false;
    this->objRight = (this->avgDistRight < PROXIMITY_TH) ? true : false;
    std::cout<<"Object Left: "<< this->objLeft <<std::endl;
    std::cout<<"Object Middle: "<< this->objMiddle <<std::endl;
    std::cout<<"Object Right: "<< this->objRight <<std::endl;
}

void DETECTION:: motorControl() 
{
    if ( this->objMiddle && (!this->objRight || this->objLeft) ){
        // Turn on LEFT MOTOR
        std::cout<<"Turn on LEFT MOTOR "<<std::endl;
    }
    if ( this->objMiddle && (!this->objLeft || this->objRight) ){
        // Turn on RIGHT MOTOR
        std::cout<<"Turn on RIGHT MOTOR "<<std::endl;
    }
}
*/

void DETECTION:: shadeImage(Mat roiLeft, Mat roiMiddle, Mat roiRight) 
{
    cv::Scalar green = cv::Scalar(0, 255, 0);
    cv::Scalar red = cv::Scalar(0, 0, 255);
    cv::Scalar yellow = cv::Scalar(0, 255, 255);
    double alpha = 0.15;

    if (this->objLeft == FO_CLEAR) {    
        cv::Mat color(roiLeft.size(), CV_8UC3, green);    
        cv::addWeighted(color, alpha, roiLeft, 1.0 - alpha , 0.0, roiLeft); 
    } else if (this->objLeft == FO_BLOCKED) {
        cv::Mat color(roiLeft.size(), CV_8UC3, red); 
        cv::addWeighted(color, alpha, roiLeft, 1.0 - alpha , 0.0, roiLeft); 
    } else {
        cv::Mat color(roiLeft.size(), CV_8UC3, yellow);
        cv::addWeighted(color, alpha, roiLeft, 1.0 - alpha, 0.0, roiLeft);
    }

    if (this->objMiddle == FO_CLEAR) {    
        cv::Mat color(roiMiddle.size(), CV_8UC3, green);    
        cv::addWeighted(color, alpha, roiMiddle, 1.0 - alpha , 0.0, roiMiddle); 
    } else if ( this->objMiddle == FO_BLOCKED) {
        cv::Mat color(roiMiddle.size(), CV_8UC3, red); 
        cv::addWeighted(color, alpha, roiMiddle, 1.0 - alpha , 0.0, roiMiddle); 
    } else {
        cv::Mat color(roiMiddle.size(), CV_8UC3, yellow);
        cv::addWeighted(color, alpha, roiMiddle, 1.0 - alpha, 0.0, roiMiddle);
    }

    if (this->objRight == FO_CLEAR) {    
        cv::Mat color(roiRight.size(), CV_8UC3, green);    
        cv::addWeighted(color, alpha, roiRight, 1.0 - alpha , 0.0, roiRight); 
    } else if ( this->objRight == FO_BLOCKED) {
        cv::Mat color(roiRight.size(), CV_8UC3, red); 
        cv::addWeighted(color, alpha, roiRight, 1.0 - alpha , 0.0, roiRight); 
    } else {
        cv::Mat color(roiRight.size(), CV_8UC3, yellow);
        cv::addWeighted(color, alpha, roiRight, 1.0 - alpha, 0.0, roiRight);
    }
}