#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    // create new file dialog object
    m_fileDialog = new QFileDialog( this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_m_rightDevButton_clicked()
{
    // show all file types
    m_fileDialog->setNameFilter(tr("All files (*)"));

    if ( m_fileDialog->exec())
    {
        m_rightDevPath = m_fileDialog->selectedFiles();
        ui->m_rightDevText->setText( m_rightDevPath.at(0));
    }
}


void MainWindow::on_m_leftDevButton_clicked()
{
    // show all file types
    m_fileDialog->setNameFilter(tr("All files (*)"));

    if ( m_fileDialog->exec())
    {
        m_leftDevPath = m_fileDialog->selectedFiles();
        ui->m_leftDevText->setText( m_leftDevPath.at(0));
    }
}



void MainWindow::on_m_calibFileButton_clicked()
{
    // only show .yml files
    m_fileDialog->setNameFilter(tr("YML files (*.yml *.YML)"));

    if ( m_fileDialog->exec())
    {
        m_calibFilePath = m_fileDialog->selectedFiles();
        ui->m_calibFileText->setText( m_calibFilePath.at(0));
    }
}



void MainWindow::on_m_startButton_clicked()
{
    Mat h_leftFrame;
    VideoCapture left;

    left.open( m_leftDevPath.at(0).toLocal8Bit().constData());
    if ( !left.isOpened())
    {
        // TBD: handle error
        cout << "Error: VideoCapture::open()" << endl;
        exit(EXIT_FAILURE);
    }

    while( m_bCapture)
    {
        left.grab();
        left.retrieve( h_leftFrame);

        if ( waitKey(30) >= 0)
            break;
    }

    left.release();

}
