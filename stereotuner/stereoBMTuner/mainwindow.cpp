#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);



    // create new file dialog object
    // TBD: bug workaround requires nullptr being passed to consuctor instead of pointer to parent
    m_fileDialog = new QFileDialog(nullptr);

    bm = cuda::createStereoBM();
}

MainWindow::~MainWindow()
{
    free( m_fileDialog);
    delete ui;
}

void MainWindow::on_m_rightDevButton_clicked()
{
    // show any file type
    m_fileDialog->setNameFilter(tr("Any file type (*)"));
    m_fileDialog->setDirectory( QDir("/dev"));

    if ( m_fileDialog->exec())
    {
        m_rightDevPath = m_fileDialog->selectedFiles();
        ui->m_rightDevText->setText( m_rightDevPath.at(0).toLocal8Bit().constData());
    }
}



void MainWindow::on_m_leftDevButton_clicked()
{
    // show any file type
    m_fileDialog->setNameFilter(tr("Any file type (*)"));
    m_fileDialog->setDirectory( QDir("/dev"));

    if ( m_fileDialog->exec())
    {
        m_leftDevPath = m_fileDialog->selectedFiles();
        ui->m_leftDevText->setText( m_leftDevPath.at(0).toLocal8Bit().constData());
    }
}



void MainWindow::on_m_calibFileButton_clicked()
{
    // show only .yml file type
    m_fileDialog->setNameFilter(tr("YML files (*.yml *.YML)"));
    m_fileDialog->setDirectory( QDir("/home"));

    if ( m_fileDialog->exec())
    {
        m_calibFilePath = m_fileDialog->selectedFiles();
        ui->m_calibFileText->setText( m_calibFilePath.at(0).toLocal8Bit().constData());
    }
}




void MainWindow::on_m_startButton_clicked()
{
    Mat h_rightFrame, h_leftFrame, h_rightRemap, h_leftRemap, h_disp, h_disp8;
    Mat rmap[2][2];
    cuda::GpuMat d_rightFrame, d_leftFrame, d_disp;
    Mat leftCameraMatrix, rightCameraMatrix, leftDistortionCoeffs, rightDistortionCoeffs;
    Mat R1, R2, P1, P2;
    FileStorage fs;
    VideoCapture right, left;

    m_bCapture = true;

    // open calibration file
    fs.open( m_calibFilePath.at(0).toLocal8Bit().constData(), FileStorage::READ);
    if ( !fs.isOpened())
    {
        // handle error
        cout << "Error: FileStorage::open" << endl;
        exit(EXIT_FAILURE);
    }
    // load calibration matrices
    fs["leftCameraMatrix"] >> leftCameraMatrix;
    fs["rightCameraMatrix"] >> rightCameraMatrix;
    fs["leftDistortionCoeffs"] >> leftDistortionCoeffs;
    fs["rightDistortionCoeffs"] >> rightDistortionCoeffs;
    fs["R1"] >> R1;
    fs["R2"] >> R2;
    fs["P1"] >> P1;
    fs["P2"] >> P2;

    /* generate remap matrices */
    initUndistortRectifyMap( leftCameraMatrix, leftDistortionCoeffs, R1, P1,
                             Size(1280, 720), CV_16SC2, rmap[0][0], rmap[0][1]);
    initUndistortRectifyMap( rightCameraMatrix, rightDistortionCoeffs, R2, P2,
                             Size(1280,720), CV_16SC2, rmap[1][0], rmap[1][1]);


    // open right camera
    right.open( m_rightDevPath.at(0).toLocal8Bit().constData());
    if ( !right.isOpened())
    {
        // handle error
        cout << "Error: VideoCapture::open" << endl;
        exit(EXIT_FAILURE);
    }
    // set right video device capture properties
    right.set(CAP_PROP_FRAME_WIDTH, 1280);
    right.set(CAP_PROP_FRAME_HEIGHT, 720);
    right.set(CAP_PROP_FPS, 30);
    right.set(CAP_PROP_FOURCC, VideoWriter::fourcc('M','J','P','G'));



    // open left camera
    left.open( m_leftDevPath.at(0).toLocal8Bit().constData());
    if ( !left.isOpened())
    {
        // handle error
        cout << "Error: VideoCapture::open" << endl;
        exit(EXIT_FAILURE);
    }
    // set left video device capture properties
    left.set(CAP_PROP_FRAME_WIDTH, 1280);
    left.set(CAP_PROP_FRAME_HEIGHT, 720);
    left.set(CAP_PROP_FPS, 30);
    left.set(CAP_PROP_FOURCC, VideoWriter::fourcc('M','J','P','G'));



    while( m_bCapture)
    {
        // grab frames
        right.grab();
        left.grab();

        // decode frames
        left.retrieve( h_leftFrame);
        right.retrieve( h_rightFrame);


        cvtColor( h_rightFrame, h_rightFrame, COLOR_BGR2GRAY);
        cvtColor( h_leftFrame, h_leftFrame, COLOR_BGR2GRAY);

        remap( h_leftFrame, h_leftRemap, rmap[0][0], rmap[0][1], INTER_LINEAR);
        remap( h_rightFrame, h_rightRemap, rmap[1][0], rmap[1][1], INTER_LINEAR);

        d_leftFrame.upload( h_leftRemap);
        d_rightFrame.upload( h_rightRemap);

        bm->compute( d_leftFrame, d_rightFrame, d_disp);
        d_disp.download( h_disp);
        normalize( h_disp, h_disp8, 0, 255, CV_MINMAX, CV_8U);

        imshow("Disparity Map", h_disp8);

       if ( waitKey(30) >= 0)
           break;
    }

    destroyWindow("Disparity Map");
    right.release();
}

void MainWindow::on_m_quitButton_clicked()
{
    m_bCapture = false;
}

void MainWindow::on_m_blockSizeSlider_sliderMoved(int position)
{
    if ( position % 2 == 0)
        position += 1;

    bm->setBlockSize( position);
}

void MainWindow::on_m_minDisparitySlider_sliderMoved(int position)
{
    bm->setMinDisparity( position);
}



void MainWindow::on_m_numDisparitiesSlider_sliderMoved(int position)
{
    bm->setNumDisparities( (position/8)*8);
}

void MainWindow::on_m_speckleRangeSlider_sliderMoved(int position)
{
    bm->setSpeckleRange( position);
}

void MainWindow::on_m_speckleWinSizeSlider_sliderMoved(int position)
{
    bm->setSpeckleWindowSize(position);
}

void MainWindow::on_m_textureThreshSlider_sliderMoved(int position)
{
    bm->setTextureThreshold( position);
}

void MainWindow::on_m_uniqueRatioSlider_sliderMoved(int position)
{
    bm->setUniquenessRatio( position);
}

void MainWindow::on_m_preFilterCapSlider_sliderMoved(int position)
{
    bm->setPreFilterCap( position);
}

void MainWindow::on_m_preFilterSizeSlider_sliderMoved(int position)
{
    bm->setPreFilterSize( position);
}
