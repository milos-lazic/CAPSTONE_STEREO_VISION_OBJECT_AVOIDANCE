// #include <stdio.h>
// #include <stdlib.h>
// #include <cuda_runtime.h>
// #include "cm/cmproto.h"
#include <sys/time.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <getopt.h>

#include "opencv2/core.hpp"
#include "opencv2/core/types_c.h"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/cudafilters.hpp"
#include "opencv2/cudaimgproc.hpp"
#include "opencv2/cudastereo.hpp"

#include "CApp.h"


using namespace std;
using namespace cv;


#define CLEAR(x) \
	memset(&x, 0, sizeof(x))



enum e_long_opt
{
	OPT_CAPTURE = 0,     /* capture images */
	OPT_CALIBRATE,       /* calibrate cameras */
	OPT_LDIR,            /* left image directory */
	OPT_RDIR,            /* right image directory */
	OPT_LDEV,            /* left camera device file */
	OPT_RDEV,            /* right camera device file */
	OPT_MAX,
};



void showHelpAndExit( char *prog_name)
{
	cout << "\nUsage:\n " << prog_name << "\n"
	"\t[--capture]\n" 
	"\t[--calibrate]\n" 
	"\t[--ldir=<path_to_left_image_directory>]\n" 
	"\t[--rdir=<path_to_right_image_directory>]\n"
	"\t[--ldev=<path_to_left_camera_device_file>]\n"
	"\t[--rdev=<path_to_right_camera_device_file>]\n"
	"\n" << endl;


	exit(EXIT_FAILURE);
}



int main( int argc, char *argv[])
{
	int s;
	int option_index;
	static struct option long_options[] = 
	{
		{"capture",    no_argument,          NULL,   OPT_CAPTURE},
		{"calibrate",  no_argument,          NULL,   OPT_CALIBRATE},
		{"ldir",       required_argument,    NULL,   OPT_LDIR},
		{"rdir",       required_argument,    NULL,   OPT_RDIR},
		{"ldev",       required_argument,    NULL,   OPT_LDEV},
		{"rdev",       required_argument,    NULL,   OPT_RDEV},
		{0,            0,            0,      0},
	};

	CApp application;

	// parse command line options 
	while(1)
	{
		s = getopt_long( argc, argv, "", long_options, &option_index);

		if ( s == -1)
		{
			// all options have been parsed so break from while loop
			break;
		}

		switch(s)
		{
			case OPT_CAPTURE:
				application.setCapture(true);
				break;

			case OPT_CALIBRATE:
				application.setCalibrate(true);
				break;

			case OPT_LDIR:
				application.setLeftImageDir(optarg);
				break;

			case OPT_RDIR:
				application.setRightImageDir(optarg);
				break;

			case OPT_LDEV:
				break;

			case OPT_RDEV:
				break;

			default:
				// invalid option, print usage help
				showHelpAndExit( argv[0]);
				break;
		}
	}

	application.showSettings();
	application.workBegin();


	return 0;
}
