#include "CApp.h"




void CApp::stateFxn_Capture( void)
{
	int count = 0;
	char img_name[256], absPath[PATH_MAX];
	XMLDocument xmlDoc;
	XMLNode * pRoot;
	XMLElement *pImageList, *pImagePair, *pRight, *pLeft;
	Mat left_img_src, right_img_src;
	Mat left_img_gray, right_img_gray;
	Mat win_mat(Size(width*2, height), CV_8UC1);


	// open right camera device file and configure stream format
	video_right.open( right_device_file.c_str());
	if ( !video_right.isOpened())
	{
		// handle error
		exit(EXIT_FAILURE);
	}

	video_right.set(CAP_PROP_FRAME_WIDTH, width);
	video_right.set(CAP_PROP_FRAME_HEIGHT, height);
	video_right.set(CAP_PROP_FPS, 30);
	video_right.set(CAP_PROP_FOURCC, VideoWriter::fourcc('M','J','P','G'));
	//video_right.set(CAP_PROP_FOURCC, VideoWriter::fourcc('Y','U','Y','V'));


	// open left camera device file and configure stream format 
	video_left.open( left_device_file.c_str());
	if ( !video_left.isOpened())
	{
		// handle error
		exit(EXIT_FAILURE);
	}

	video_left.set(CAP_PROP_FRAME_WIDTH, width);
	video_left.set(CAP_PROP_FRAME_HEIGHT, height);
	video_left.set(CAP_PROP_FPS, 30);
	video_left.set(CAP_PROP_FOURCC, VideoWriter::fourcc('M','J','P','G'));
	//video_left.set(CAP_PROP_FOURCC, VideoWriter::fourcc('Y','U','Y','V'));


	/* TBD: print message so user knows what keys to press to capture image/quit, etc... */
	cout << "\n===== CALIBRATION IMAGE CAPTURE ====="
	"\n\t1) Position chessboard so that it is visible in both frames"
	"\n\t2) Press 'c' key to capture and store calibration pair"
	"\n\t3) Press 'q' to quit image capturing"
	"\n" << endl;


	
	/* start XML document with image pair paths */
	pRoot = xmlDoc.NewElement("Root");
	xmlDoc.InsertFirstChild(pRoot);

	pImageList = xmlDoc.NewElement("ImageList");
	pRoot->InsertFirstChild(pImageList);


	/* capture (up to) NUM_IMGS calibration pairs */
	while( count < this->num_images)
	{
		/* grab frame buffers before decoding; required if camera
		   lacks hardware synchronization */
		video_right.grab();
		video_left.grab();

		/* decode frames */
		video_right.retrieve( right_img_src);
		video_left.retrieve( left_img_src);

		/* convert images to grayscale */
		cvtColor( right_img_src, right_img_gray, COLOR_BGR2GRAY);
		cvtColor( left_img_src, left_img_gray, COLOR_BGR2GRAY);

		/* copy images to output window buffer */
		left_img_gray.copyTo( win_mat(Rect(0, 0, width, height)));
		right_img_gray.copyTo( win_mat(Rect(width, 0, width, height)));

		/* show images */
		imshow("Capture", win_mat);


		switch( waitKey(30))
		{
			case 'c':
				pImagePair = xmlDoc.NewElement("ImagePair");
				pImageList->InsertEndChild(pImagePair);


				// generate left image file name
				snprintf( img_name, sizeof(img_name)-1, "%sleft%d.jpg",
					this->left_img_dir.c_str(), count);

				(void) realpath( img_name, absPath); // get absolute path

				// save left frame
				imwrite( absPath, left_img_gray);

				pLeft = xmlDoc.NewElement("Left");
				pLeft->SetText( absPath);
				pImagePair->InsertEndChild(pLeft);



				// generate right image file name
				snprintf( img_name, sizeof(img_name)-1, "%sright%d.jpg",
					this->right_img_dir.c_str(), count);

				(void) realpath( img_name, absPath); // get absolute path

				// save right frame
				imwrite( absPath, right_img_gray);

				pRight = xmlDoc.NewElement("Right");
				pRight->SetText( absPath);
				pImagePair->InsertEndChild(pRight);

				count++; // increment image counter
				break;

			case 'q':
				// quit application
				/* TBD: for now just exit; this should return an event to the
				        application state machine that indicates it cannot progress 
				        and the app should be quit */
				exit(EXIT_FAILURE);
				break;

			default:
				// do nothing; continue
				break;
		}
	}

	video_right.release();
	video_left.release();

	xmlDoc.SaveFile("stereo_calib.xml"); // save XML file
}


bool CApp::runCalibrationAndSave( vector<string>& imgList,
		                          Size chessBoardSize,
		                          string outFileName)
{
	vector<string>::iterator it;
	Mat img, cameraMatrix, distortionCoeffs;
	vector<Mat> rvecs, tvecs;
	int flag = CALIB_FIX_K4 | CALIB_FIX_K5;
	bool bFoundCorners;
	vector< vector< Point3f > > object_points; // vector of vectors to store 3D coordinates of chessboard corners
	vector< vector< Point2f > > image_points; // vector of vectors to store 2D pixel coordinates of corners
	vector< Point2f > corners;
	vector< Point3f > obj;


	// loop through all images in image list
	it = imgList.begin();
	while( it != imgList.end())
	{
		img = imread( *it, CV_LOAD_IMAGE_GRAYSCALE);
		if ( img.empty())
		{
			fprintf(stderr, "Error: imread(), line %d\n", __LINE__);
		}
		else
		{
			bFoundCorners = findChessboardCorners( img, chessBoardSize, corners,
			CALIB_CB_FILTER_QUADS | CALIB_CB_ADAPTIVE_THRESH);

			if ( bFoundCorners)
			{
				printf("Found corners in image [%d]\n", int(it-imgList.begin()));

				cornerSubPix( img, corners, Size(5, 5), Size(-1, -1),
					TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));

#if 0			/* optionally draw chessboard corners */
				drawChessboardCorners( img, chessBoardSize, corners, bFoundCorners);
				imshow("Image Corners", img);
				waitKey(0);
#endif

				// load object points vector
				for ( int i = 0; i < BOARD_HEIGHT; i++)
					for( int j = 0; j < BOARD_WIDTH; j++)
						obj.push_back( Point3f( float(j*SQUARE_SIZE), float(i*SQUARE_SIZE), 0));

				image_points.push_back(corners);
				object_points.push_back(obj); obj.clear();
			}
		}

		it++;
	}


	// make sure there are at least 10 good calibration images
	if ( image_points.size() < 10)
	{
		return false;
	}
	// else do the calibration
	else
	{
		calibrateCamera( object_points, image_points, img.size(), cameraMatrix,
			distortionCoeffs, rvecs, tvecs, flag);

		// save the calibration matrices
		FileStorage fs( outFileName.c_str(), FileStorage::WRITE);
		fs << "cameraMatrix" << cameraMatrix;
		fs << "distortionCoeffs" << distortionCoeffs;
	}

	printf("Camera calibration done...\n");

	return true;
}



void CApp::stateFxn_CalibLeft( void) 
{
	XMLDocument xmlDoc;
	XMLNode * pRoot;
	XMLElement * pImageList, * pPair, *pLeft;
	vector<string> images;
	bool bCalibrated = false;

	cout << "\n===== Starting Left Camera Calibration =====" << endl;


	// load XML file containing image list
	xmlDoc.LoadFile("stereo_calib.xml");

	// get root node pointer
	pRoot = xmlDoc.FirstChild();

	pImageList = pRoot->FirstChildElement("ImageList");

	// insert each image path in vector
	pPair = pImageList->FirstChildElement("ImagePair");
	while( pPair)
	{
		pLeft = pPair->FirstChildElement("Left");
		images.push_back(pLeft->GetText());
		pPair = pPair->NextSiblingElement("ImagePair");
	}

	cout << "Found " << images.size() << " images" << endl;

	bCalibrated = runCalibrationAndSave( images, Size(9, 6), string("data/left/leftCalib.yml"));

	if ( bCalibrated)
		printf("Left camera successfully calibrated!\n");
	else
		printf("Left camera not calibrated.\n");
}



void CApp::stateFxn_CalibRight( void) 
{
	XMLDocument xmlDoc;
	XMLNode * pRoot;
	XMLElement * pImageList, * pPair, * pRight;
	vector<string> images;
	bool bCalibrated = false;

	cout << "\n===== Starting Right Camera Calibration =====" << endl;

	// load XML document containin image paths
	xmlDoc.LoadFile("stereo_calib.xml");

	// get pointer to root node
	pRoot = xmlDoc.FirstChild();

	pImageList = pRoot->FirstChildElement("ImageList");

	// insert each image path in vector
	pPair = pImageList->FirstChildElement("ImagePair");
	while( pPair)
	{
		pRight = pPair->FirstChildElement("Right");
		images.push_back(pRight->GetText());
		pPair = pPair->NextSiblingElement("ImagePair");
	}

	cout << "Found " << images.size() << " images" << endl;

	bCalibrated = runCalibrationAndSave( images, Size(9, 6), string("data/right/rightCalib.yml"));

	if ( bCalibrated)
		printf("Right camera successfully calibrated!\n");
	else
		printf("Right camera not calibrated.\n");
	
}


void CApp::stateFxn_CalibStereo( void) 
{
	FileStorage fsl, fsr, fsOut;
	vector< vector< Point3f>> object_points;
	vector< vector< Point2f>> left_image_points, right_image_points;
	vector< Point3f> obj;
	vector< Point2f> leftCorners, rightCorners;
	bool bFoundLeftCorners, bFoundRightCorners;
	XMLDocument xmlDoc;
	XMLNode * pRoot;
	XMLElement * pImageList, * pPair, * pLeft, * pRight;
	Mat leftFrame, rightFrame;
	Mat leftCameraMatrix, rightCameraMatrix;
	Mat leftDistortionCoeffs, rightDistortionCoeffs;
	Mat R, F, E, R1, R2, P1, P2, Q;
	Vec3d T;

	printf("Starting stereo camera calibration\n");

	// load XML file containing image list
	xmlDoc.LoadFile("stereo_calib.xml"); // TBD: replace hard-coded file path
	// TBD: error checking

	// get root node pointer
	pRoot = xmlDoc.FirstChild();
	// TBD: error checking;

	// get pointer to start of image list
	pImageList = pRoot->FirstChildElement("ImageList");
	// TBD: error checking

	// get pointer to first image pair
	pPair = pImageList->FirstChildElement("ImagePair");
	// TBD: error checking
	while( pPair)
	{
		// get pointer to left image entry
		pLeft = pPair->FirstChildElement("Left");
		// TBD: error checking

		// get pointer to right image entry
		pRight = pPair->FirstChildElement("Right");
		// TBD: error checking

		// open image pair
		leftFrame = imread( pLeft->GetText(), CV_LOAD_IMAGE_GRAYSCALE);
		rightFrame = imread( pRight->GetText(), CV_LOAD_IMAGE_GRAYSCALE);
		// if unable to open images
		if ( leftFrame.empty() || rightFrame.empty())
		{
			fprintf(stderr, "Error: imread(), line %d\n", __LINE__);
		}
		// else try tofind image corners and object points
		else
		{
			// find corners in left image
			// TBD: remove hard-coded Size argument
			bFoundLeftCorners = findChessboardCorners( leftFrame,
				                                       Size(9,6),
				                                       leftCorners,
				                                       CALIB_CB_FILTER_QUADS | CALIB_CB_ADAPTIVE_THRESH);

			// find corners in right image
			// TBD: remove hard-coded Size argument
			bFoundRightCorners = findChessboardCorners( rightFrame,
				                                        Size(9,6),
				                                        rightCorners,
				                                        CALIB_CB_FILTER_QUADS | CALIB_CB_ADAPTIVE_THRESH);


			// if corners found in both frames
			if ( bFoundLeftCorners && bFoundRightCorners)
			{

				cornerSubPix( leftFrame, leftCorners, Size(5,5), Size(-1,-1),
					TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));

				cornerSubPix( rightFrame, rightCorners, Size(5,5), Size(-1,-1),
					TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));

				// load object points vector
				for ( int i = 0; i < BOARD_HEIGHT; i++)
					for( int j = 0; j < BOARD_WIDTH; j++)
						obj.push_back( Point3f( float(j*SQUARE_SIZE), float(i*SQUARE_SIZE), 0));

				
				left_image_points.push_back( leftCorners);
				right_image_points.push_back( rightCorners);
				object_points.push_back(obj); obj.clear();
			}
			else
			{
				printf("Corners not found\n");
			}
		}

		// get pointer to next image pair
		pPair = pPair->NextSiblingElement("ImagePair");
	}


	// TBD: replace hard-coded calibration file paths
	fsl.open( string("data/left/leftCalib.yml"), FileStorage::READ);
	fsr.open( string("data/right/rightCalib.yml"), FileStorage::READ);
	// check if calibration files opened successfully
	if ( !fsl.isOpened() || !fsr.isOpened())
	{
		fprintf(stderr, "Error: FileStorage::open(), line %d\n", __LINE__);
		exit(EXIT_FAILURE);
	}

	/* load intrinsic matrices from individual camera calibration */
	fsl["cameraMatrix"] >> leftCameraMatrix;
	fsl["distortionCoeffs"] >> leftDistortionCoeffs;
	fsr["cameraMatrix"] >> rightCameraMatrix;
	fsr["distortionCoeffs"] >> rightDistortionCoeffs;


	// run the stereo calibration
	stereoCalibrate( object_points, left_image_points, right_image_points,
		leftCameraMatrix, leftDistortionCoeffs, rightCameraMatrix, rightDistortionCoeffs,
		leftFrame.size(), R, T, E, F, CALIB_FIX_INTRINSIC);

	// run stereo rectification (gets rectification transforms)
	stereoRectify( leftCameraMatrix, leftDistortionCoeffs,
		           rightCameraMatrix, rightDistortionCoeffs,
		           leftFrame.size(), R, T, R1, R2, P1, P2, Q,
		           CALIB_ZERO_DISPARITY, -1, leftFrame.size());	


	// write calibration matrices to file
	// TBD replace hard-coded calibration file path
	fsOut.open("data/stereoCalib.yml", FileStorage::WRITE);
	// check if calibration file openec successfully
	if ( !fsOut.isOpened())
	{
		fprintf(stderr, "Error: FileStorage::open(), line %d\n", __LINE__);
		exit(EXIT_FAILURE);
	}

	fsOut << "leftCameraMatrix" << leftCameraMatrix;
	fsOut << "rightCameraMatrix" << rightCameraMatrix;
	fsOut << "leftDistortionCoeffs" << leftDistortionCoeffs;
	fsOut << "rightDistortionCoeffs" << rightDistortionCoeffs;
	fsOut << "R" << R; /* rotation matrix between left and right camera coordinate systems */
	fsOut << "T" << T; /* translation vector between left and right camera coordinate systems */
	fsOut << "E" << E; /* essential matrix */
	fsOut << "F" << F; /* fundamental matrix */
	fsOut << "R1" << R1; /* rotation matrix (rectification transform) for left camera */
	fsOut << "R2" << R2; /* rotation matrix (recitfication transform) for right camera */
	fsOut << "P1" << P1; /* projection matrix for left camera (also called 'new camera matrix') */
	fsOut << "P2" << P2; /* projection matrix for right camera (also called 'new camera matrix') */
	fsOut << "Q" << Q; /* disparity-to-depth mapping matrix */


	printf("Finished stereo camera calibration\n");
	
	// release memory
	leftFrame.release();
	rightFrame.release();

	// close calibration files
	fsl.release();
	fsr.release();
	fsOut.release();
}


void CApp::stateFxn_ImgCorrect( void) {}



void CApp::showSettings(void)
{
	cout << "\n===== APPLICATION SETTINGS ====="
	"\n\tCapture:             " << (bCapture ? "YES" : "NO") <<
	"\n\tCalibrate:           " << (bCalibrate ? "YES" : "NO") <<
	"\n\tLeft image dir:      " << left_img_dir <<
	"\n\tRight image dir:     " << right_img_dir <<
	"\n\tLeft device file:    " << left_device_file <<
	"\n\tRight device file:   " << right_device_file <<
	"\n" << endl;
}

using namespace tinyxml2;

void CApp::workBegin(void)
{

	//if capture option specified
	if ( bCapture)
	{
		/* start the state machine */
		this->fsmExternEvent( STATE_CAPTURE, nullptr);
	}

	stateFxn_CalibLeft();
	stateFxn_CalibRight();

	stateFxn_CalibStereo();

#if 1 // function testing

	Mat rmap[2][2];
	Mat leftFrame, rightFrame, leftRemap, rightRemap, disp, disp8;
	cuda::GpuMat d_leftFame, d_rightFrame, d_disp;
	FileStorage fs;
	Mat leftCameraMatrix, rightCameraMatrix, leftDistortionCoeffs, rightDistortionCoeffs;
	Mat R1, R2, P1, P2;

	fs.open(string("data/stereoCalib.yml"), FileStorage::READ);
	if ( !fs.isOpened())
	{
		fprintf(stderr, "Error: FileStorage::open(), line %d\n", __LINE__);
		exit(EXIT_FAILURE);
	}

	fs["leftCameraMatrix"] >> leftCameraMatrix;
	fs["rightCameraMatrix"] >> rightCameraMatrix;
	fs["leftDistortionCoeffs"] >> leftDistortionCoeffs;
	fs["rightDistortionCoeffs"] >> rightDistortionCoeffs;
	fs["R1"] >> R1;
	fs["R2"] >> R2;
	fs["P1"] >> P1;
	fs["P2"] >> P2;

	leftFrame = imread("data/left/left0.jpg");
	rightFrame = imread("data/right/right0.jpg");
	if ( leftFrame.empty() || rightFrame.empty())
	{
		fprintf(stderr, "Error: imread(), line %d\n", __LINE__);
		exit(EXIT_FAILURE);
	}

	initUndistortRectifyMap( leftCameraMatrix, leftDistortionCoeffs, R1, P1, 
		leftFrame.size(), CV_16SC2, rmap[0][0], rmap[0][1]);
	initUndistortRectifyMap( rightCameraMatrix, rightDistortionCoeffs, R2, P2,
		rightFrame.size(), CV_16SC2, rmap[1][0], rmap[1][1]);



	Ptr<cuda::StereoBM> bm;
	bm = cuda::createStereoBM();
	bm->setBlockSize(11);
	bm->setMinDisparity(4);
	bm->setNumDisparities(128);
	bm->setSpeckleRange(10);
	bm->setSpeckleWindowSize(50);
	bm->setSmallerBlockSize(10);
	bm->setTextureThreshold(15);
	bm->setUniquenessRatio(10);
	bm->setPreFilterCap(50);
	bm->setPreFilterSize(20);

	
	

	VideoCapture right, left;
	right.open("/dev/video1");
	left.open("/dev/video2");

	right.set(CAP_PROP_FRAME_WIDTH, IMG_WIDTH);
	right.set(CAP_PROP_FRAME_HEIGHT, IMG_HEIGHT);
	right.set(CAP_PROP_FPS, 30);
	right.set(CAP_PROP_FOURCC, VideoWriter::fourcc('M','J','P','G'));

	left.set(CAP_PROP_FRAME_WIDTH, IMG_WIDTH);
	left.set(CAP_PROP_FRAME_HEIGHT, IMG_HEIGHT);
	left.set(CAP_PROP_FPS, 30);
	left.set(CAP_PROP_FOURCC, VideoWriter::fourcc('M','J','P','G'));

	while(1)
	{
#if 1
		right.grab();
		left.grab();

		right.retrieve( rightFrame);
		left.retrieve( leftFrame);

		cvtColor( rightFrame, rightFrame, COLOR_BGR2GRAY);
		cvtColor( leftFrame, leftFrame, COLOR_BGR2GRAY);

		remap( leftFrame, leftRemap, rmap[0][0], rmap[0][1], INTER_LINEAR);
		remap( rightFrame, rightRemap, rmap[1][0], rmap[1][1], INTER_LINEAR);


		d_leftFame.upload( leftRemap);
		d_rightFrame.upload( rightRemap);

		bm->compute( d_leftFame, d_rightFrame, d_disp);

		d_disp.download( disp);

		normalize(disp, disp8, 0, 255, CV_MINMAX, CV_8U);

		imshow("undistorted", leftRemap);
		imshow("disparity", disp8);
		if ( waitKey(30) >= 0) break;
#endif
	}



#endif
}