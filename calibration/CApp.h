#ifndef _CAPP_H_
#define _CAPP_H_

#include <sys/time.h>
#include <limits.h>
#include <iostream>
#include <string>

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

#include "FSM.h"
#include "tinyxml2.h"

#define IMG_WIDTH    1280
#define IMG_HEIGHT   720
#define NUM_IMGS     15

#define BOARD_WIDTH 9
#define BOARD_HEIGHT 6
#define SQUARE_SIZE 38 // in mm

using namespace std;
using namespace cv;
using namespace tinyxml2;



class CApp : public FSM
{
public:
	CApp() {}    /* class constructor */
	~CApp() {}   /* class destructor */

	void setCapture( bool flag) { bCapture = flag; }
	void setCalibrate ( bool flag) { bCalibrate = flag; }
	void setLeftImageDir( const string& path) { left_img_dir.assign(path); }
	void setRightImageDir( const string& path) { right_img_dir.assign(path); }
	void workBegin(void);

	void showSettings(void);


private:
	/* command line options (default values assigned) */
	bool bCapture = false;
	bool bCalibrate = false;
	string left_img_dir = "data/left/";
	string right_img_dir = "data/right/";
	string left_device_file = "/dev/video2";
	string right_device_file = "/dev/video1";
	int width = IMG_WIDTH;
	int height = IMG_HEIGHT;
	int num_images = NUM_IMGS;

	VideoCapture video_left;
	VideoCapture video_right;


	enum E_states
	{
		STATE_CAPTURE = 0,
		STATE_CALIB_LEFT,
		STATE_CALIB_RIGHT,
		STATE_CALIB_STEREO,
		STATE_IMG_CORRECT,
		STATE_MAX,
	};


	/* state function forward declarations */
	void stateFxn_Capture( void);
	void stateFxn_CalibLeft( void);
	void stateFxn_CalibRight( void);
	void stateFxn_CalibStereo( void);
	void stateFxn_ImgCorrect( void);


	bool runCalibrationAndSave( vector<string>& imgList,
		                        Size chessBoardSize,
		                        string outFileName);


	// state map, fmsGetStateMap must be defined by classes
	// that inherit from FSM class
	const fsmState_t *fsmGetStateMap ( void)
	{
		static const fsmState_t stateMap[STATE_MAX] = 
		{
			{ reinterpret_cast<fsmStateFunc>(&CApp::stateFxn_Capture) },
			{ reinterpret_cast<fsmStateFunc>(&CApp::stateFxn_CalibLeft) },
			{ reinterpret_cast<fsmStateFunc>(&CApp::stateFxn_CalibRight) },
			{ reinterpret_cast<fsmStateFunc>(&CApp::stateFxn_CalibStereo) },
			{ reinterpret_cast<fsmStateFunc>(&CApp::stateFxn_ImgCorrect) },
		};

		return &stateMap[0];
	}


};




#endif