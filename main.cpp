#include <iostream>

#include "opencv2/core.hpp"
#include "opencv2/core/types_c.h"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/cudafilters.hpp"
#include "opencv2/cudaimgproc.hpp"
#include "opencv2/cudastereo.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgcodecs/imgcodecs_c.h"
#include "opencv2/ximgproc/disparity_filter.hpp"

#include "DETECTION.h"


#define PRINT(string) \
	printf("\n\t%s", string)



#define IMG_WIDTH      1280
#define IMG_HEIGHT     720

#define VIDEO_FPS      15

#define RIGHT_DEVICE_PATH      "/dev/video1"
#define LEFT_DEVICE_PATH       "/dev/video2"
//#define CALIB_FILE_PATH        "stereoCalib.yml"
#define CALIB_FILE_PATH        "calibration/data/stereoCalib_BACKUP_2.yml"


using namespace std;
using namespace cv;
using namespace cv::ximgproc;


Rect computeROI(Size2i src_sz, Ptr<StereoMatcher> matcher_instance)
{
    int min_disparity = matcher_instance->getMinDisparity();
    int num_disparities = matcher_instance->getNumDisparities();
    int block_size = matcher_instance->getBlockSize();

    int bs2 = block_size/2;
    int minD = min_disparity, maxD = min_disparity + num_disparities - 1;

    int xmin = maxD + bs2;
    int xmax = src_sz.width + minD - bs2;
    int ymin = bs2;
    int ymax = src_sz.height - bs2;

    Rect r(xmin, ymin, xmax - xmin, ymax - ymin);
    return r;
}




int main ( int argc, char *argv[])
{
	Mat h_rightFrameSrc, h_leftFrameSrc, h_disp, h_disp8;
	Mat h_rightFrameGray, h_leftFrameGray;
	Mat h_rightRemap, h_leftRemap;
	cuda::GpuMat d_rightFrame, d_leftFrame, d_disp;

	Mat leftCameraMatrix, rightCameraMatrix;
	Mat leftDistortionCoeffs, rightDistortionCoeffs;
	Mat R1, R2, P1, P2;
	Mat rmap[2][2];

	VideoCapture right, left;
	FileStorage calibFile;
	Ptr<cuda::StereoBM> bm;

	Rect ROI;
	Mat h_filteredDisp;
	Ptr <DisparityWLSFilter> wlsFilter;

	Mat detectImage;


/* START INITIALIZATION */

	// open calibration file
	PRINT("Opening calibration file");
	calibFile.open( CALIB_FILE_PATH, FileStorage::READ);
	if ( !calibFile.isOpened())
	{
		// error; unable to open calibration file
		fprintf(stderr, "Error(%d): FileStorage::open\n", __LINE__);
		exit(EXIT_FAILURE);
	}
	// load the calibration matrices
	calibFile["leftCameraMatrix"] >> leftCameraMatrix; PRINT("Loading left camera matrix");
	calibFile["rightCameraMatrix"] >> rightCameraMatrix; PRINT("Loading right camera matrix");
	calibFile["leftDistortionCoeffs"] >> leftDistortionCoeffs; PRINT("Loading left distortion coefficient matrix");
	calibFile["rightDistortionCoeffs"] >> rightDistortionCoeffs; PRINT("Loading right distortion coefficient matrix");
	calibFile["R1"] >> R1; PRINT("Loading R1 matrix");
	calibFile["R2"] >> R2; PRINT("Loading R2 matrix");
	calibFile["P1"] >> P1; PRINT("Loading P1 matrix");
	calibFile["P2"] >> P2; PRINT("Loading P2 matrix");

	
	// generate remap matrices
	PRINT("Generating remap matrices");
	initUndistortRectifyMap( leftCameraMatrix, leftDistortionCoeffs, R1, P1,
		                     Size(IMG_WIDTH, IMG_HEIGHT), CV_16SC2, rmap[0][0], rmap[0][1]);

	initUndistortRectifyMap( rightCameraMatrix, rightDistortionCoeffs, R2, P2,
		                     Size(IMG_WIDTH, IMG_HEIGHT), CV_16SC2, rmap[1][0], rmap[1][1]);


	// open right video device
	PRINT("Opening right video device");
	right.open( RIGHT_DEVICE_PATH);
	if ( !right.isOpened())
	{
		// error: unable to open right device
		fprintf(stderr, "Error(%d): VideoCapture::open\n", __LINE__);
		exit(EXIT_FAILURE);
	}
	// set right video device capture properties
	right.set(CAP_PROP_FRAME_WIDTH, IMG_WIDTH);
	right.set(CAP_PROP_FRAME_HEIGHT, IMG_HEIGHT);
	right.set(CAP_PROP_FPS, VIDEO_FPS);
	right.set(CAP_PROP_FOURCC, VideoWriter::fourcc('M', 'J', 'P', 'G'));



	// open left video device
	PRINT("Opening left video device");
	left.open( LEFT_DEVICE_PATH);
	if ( !left.isOpened())
	{
		// error: unable to open left device
		fprintf(stderr, "Error(%d): VideoCapture::open\n", __LINE__);
		exit(EXIT_FAILURE);
	}
	// set left video device capture properties
	left.set( CAP_PROP_FRAME_WIDTH, IMG_WIDTH);
	left.set( CAP_PROP_FRAME_HEIGHT, IMG_HEIGHT);
	left.set( CAP_PROP_FPS, VIDEO_FPS);
	left.set( CAP_PROP_FOURCC, VideoWriter::fourcc('M', 'J', 'P', 'G'));


	// create instance of Stere BM solver
	bm = cuda::createStereoBM();
	bm->setBlockSize(21);
	bm->setMinDisparity(2);
	bm->setNumDisparities(128);
	bm->setSpeckleRange(10);
	bm->setSpeckleWindowSize(50);
	bm->setSmallerBlockSize(10);
	bm->setTextureThreshold(2);
	bm->setUniquenessRatio(15);
	bm->setPreFilterCap(50);
	bm->setPreFilterSize(20);


	// create instance of WLS filter
	wlsFilter = createDisparityWLSFilterGeneric(false);
	wlsFilter->setDepthDiscontinuityRadius((int)ceil(0.33*(IMG_WIDTH*IMG_HEIGHT)));
	wlsFilter->setLambda(8000.0);
	wlsFilter->setSigmaColor(1.5);
	//wlsFilter->setLRCthresh(1000);

/* STOP INITIALIZATION */

	// enter capture loop
	PRINT("Starting capture\n");
	while(true)
	{
		/* NOTE: decoding step is more expensive than grabbing,
		         so for the best synchronization possible we should
		         both frames first (remove frame buffer from queue)
		         then decode them!
		 */

		// grab frames
		right.grab();
		left.grab();

		// decode frames
		right.retrieve( h_rightFrameSrc);
		left.retrieve( h_leftFrameSrc);

		// convert frames to grayscale
		cvtColor( h_rightFrameSrc, h_rightFrameGray, COLOR_BGR2GRAY);
		cvtColor( h_leftFrameSrc, h_leftFrameGray, COLOR_BGR2GRAY);

		// remap (undistort) frames
		remap( h_rightFrameGray, h_rightRemap, rmap[1][0], rmap[1][1], INTER_LINEAR);
		remap( h_leftFrameGray, h_leftRemap, rmap[0][0], rmap[0][1], INTER_LINEAR);		

		// upload remapped frames to device (GPU) memory
		d_leftFrame.upload( h_leftRemap);
		d_rightFrame.upload( h_rightRemap);

		// compute the disparity map
		bm->compute( d_leftFrame, d_rightFrame, d_disp);

		d_disp.download( h_disp);
		//normalize( h_disp, h_disp8, 0, 255, CV_MINMAX, CV_8U);


		ROI = computeROI( h_leftFrameGray.size(), bm);
		wlsFilter->filter( h_disp, h_leftRemap, h_filteredDisp, Mat(), ROI);

		// imshow("Right Frame Source", h_rightFrameGray);
		// imshow("Right Frame Remap", h_rightRemap);
		// imshow("Disparity map", h_filteredDisp);

		// convert filtered disparity map to 3 channel color space
		cvtColor( h_filteredDisp, h_filteredDisp, COLOR_GRAY2BGR);

		// specify cropping region of interest
		Rect cropROI ( 137, 10, 1133, 700);
		detectImage = h_filteredDisp(cropROI);
		DETECTION detection( detectImage);

		// show cropped disparity map
		// imshow("Disparity map", detectImage);

		if ( waitKey(30) >= 0)
			break;
	}



	right.release();
	left.release();

	return 0;
}