#-------------------------------------------------
#
# Project created by QtCreator 2018-05-31T14:50:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = stereoBMTuner
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp

HEADERS += \
        mainwindow.h

FORMS += \
        mainwindow.ui

INCLUDEPATH = "/usr/local/include"

LIBS += -L/usr/local/lib -lopencv_cudastereo
LIBS += -L/usr/local/lib -lopencv_videostab
LIBS += -L/usr/local/lib -lopencv_photo
LIBS += -L/usr/local/lib -lopencv_stitching
LIBS += -L/usr/local/lib -lopencv_shape
LIBS += -L/usr/local/lib -lopencv_cudaobjdetect
LIBS += -L/usr/local/lib -lopencv_dnn
LIBS += -L/usr/local/lib -lopencv_superres
LIBS += -L/usr/local/lib -lopencv_cudaoptflow
LIBS += -L/usr/local/lib -lopencv_cudafeatures2d
LIBS += -L/usr/local/lib -lopencv_cudacodec
LIBS += -L/usr/local/lib -lopencv_cudabgsegm
LIBS += -L/usr/local/lib -lopencv_cudalegacy
LIBS += -L/usr/local/lib -lopencv_calib3d
LIBS += -L/usr/local/lib -lopencv_features2d
LIBS += -L/usr/local/lib -lopencv_highgui
LIBS += -L/usr/local/lib -lopencv_videoio
LIBS += -L/usr/local/lib -lopencv_imgcodecs
LIBS += -L/usr/local/lib -lopencv_cudaimgproc
LIBS += -L/usr/local/lib -lopencv_cudafilters
LIBS += -L/usr/local/lib -lopencv_video
LIBS += -L/usr/local/lib -lopencv_objdetect
LIBS += -L/usr/local/lib -lopencv_flann
LIBS += -L/usr/local/lib -lopencv_cudaarithm
LIBS += -L/usr/local/lib -lopencv_ml
LIBS += -L/usr/local/lib -lopencv_cudawarping
LIBS += -L/usr/local/lib -lopencv_imgproc
LIBS += -L/usr/local/lib -lopencv_core
LIBS += -L/usr/local/lib -lopencv_cudev
