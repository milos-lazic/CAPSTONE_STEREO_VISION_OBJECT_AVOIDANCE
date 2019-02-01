#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>

#include <QMainWindow>
#include <QFileDialog>

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

using namespace std;
using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_m_rightDevButton_clicked();

    void on_m_startButton_clicked();

    void on_m_leftDevButton_clicked();

    void on_m_calibFileButton_clicked();

    void on_m_quitButton_clicked();

    void on_m_blockSizeSlider_sliderMoved(int position);

    void on_m_minDisparitySlider_sliderMoved(int position);


    void on_m_numDisparitiesSlider_sliderMoved(int position);

    void on_m_speckleRangeSlider_sliderMoved(int position);

    void on_m_speckleWinSizeSlider_sliderMoved(int position);

    void on_m_textureThreshSlider_sliderMoved(int position);

    void on_m_uniqueRatioSlider_sliderMoved(int position);

    void on_m_preFilterCapSlider_sliderMoved(int position);

    void on_m_preFilterSizeSlider_sliderMoved(int position);

private:
    Ui::MainWindow *ui;

    bool m_bCapture = true;

    QFileDialog  * m_fileDialog;

    QStringList  m_rightDevPath;
    QStringList  m_leftDevPath;
    QStringList  m_calibFilePath;

    Ptr<cuda::StereoBM> bm;
};

#endif // MAINWINDOW_H
