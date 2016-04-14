#-------------------------------------------------
#
# Project created by QtCreator 2016-03-09T21:29:58
#
#-------------------------------------------------

QT       += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = StartWindow
TEMPLATE = app


SOURCES += main.cpp\
        startwindow.cpp \
    multiserver.cpp \
    audiomanager.cpp \
    audiothread.cpp \
    networkmanager.cpp \
    circularbuffer.cpp \
    networkaudioplayer.cpp \
    peertopeer.cpp \
    multiclient.cpp \
    audioplaythread.cpp \
    microphonemanager.cpp \
    incomingconnthread.cpp \
    udpsendthread.cpp \
    audiorecordthread.cpp \
    localaudiomanager.cpp \
    filemanager.cpp \
    filereader.cpp

HEADERS  += startwindow.h \
    multiserver.h \
    audiomanager.h \
    audiothread.h \
    networkmanager.h \
    circularbuffer.h \
    networkaudioplayer.h \
    peertopeer.h \
    multiclient.h \
    audioplaythread.h \
    microphonemanager.h \
    incomingconnthread.h \
    udpsendthread.h \
    audiorecordthread.h \
    localaudiomanager.h \
    wavheader.h \
    filemanager.h \
    filereader.h

FORMS    += startwindow.ui \
    multiserver.ui \
    peertopeer.ui \
    multiclient.ui

LIBS += -lws2_32
