#ifndef DETECTION_H
#define DETECTION_H

#include "FLC.h"
#include "clientsocket.h"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <string>

using namespace cv;
using namespace std;



class DETECTION
{
public:
	DETECTION(Mat image);
	~DETECTION();

    void getDistance();
    void thresholdDetection();
    void motorControl();
    void shadeImage(Mat roiLeft, Mat roiMiddle, Mat roiRight);

    int minValue(string direction);
    int maxValue(string direction);
private:
    Mat detectImage;
    
    flcOuput_t objLeft;
    flcOuput_t objMiddle;
    flcOuput_t objRight;

    double minLeft;
    double minMiddle;
    double minRight;

    double maxLeft;
    double maxMiddle;
    double maxRight;

    double meanLeft;
    double meanMiddle;
    double meanRight;

    double avgDistLeft;
    double avgDistMiddle;
    double avgDistRight;

    double minDistLeft;
    double minDistMiddle;
    double minDistRight;

    vector<double> stdDevLeft;
    vector<double> stdDevMiddle;
    vector<double> stdDevRight;
    vector<double> dummyVec;


    int imageHeight;
    int imageWdith;
};
#endif