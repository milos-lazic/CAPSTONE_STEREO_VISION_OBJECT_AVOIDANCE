#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>
#include <QMainWindow>
#include <QFileDialog>
#include <QPlainTextEdit>

#include "opencv2/highgui.hpp"


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

    void on_m_leftDevButton_clicked();

    void on_m_calibFileButton_clicked();

    void on_m_startButton_clicked();

private:
    Ui::MainWindow *ui;

    bool m_bCapture = true;

    QStringList m_rightDevPath; // right camera interface device file path
    QStringList m_leftDevPath;  // left camera interface device file path
    QStringList m_calibFilePath; // calibration file path

    QFileDialog * m_fileDialog; // file dialog
};

#endif // MAINWINDOW_H
