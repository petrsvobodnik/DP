#-------------------------------------------------
#
# Project created by QtCreator 2017-12-21T08:32:36
#
#-------------------------------------------------

QT       += core gui printsupport serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = hack_connect
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -lhackrf # tento zkurveny radek se musi vlozit, aby to jelo
LIBS += /usr/local/lib/libfftw3.a -lfftw3f
LIBS += -lqwt-qt5

INCLUDEPATH += /usr/include/qwt

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    freqsetting.cpp \
    qcustomplot.cpp \
    rfusetting.cpp \
    zerospan.cpp \
    loaddata.cpp

HEADERS += \
        mainwindow.h \
        hackrf.h \
    freqsetting.h \
    qcustomplot.h \
    rfusetting.h \
    zerospan.h \
    loaddata.h

FORMS += \
        mainwindow.ui \
    freqsetting.ui \
    rfusetting.ui \
    zerospan.ui \
    loaddata.ui

DISTFILES += \
    ../../Downloads/od rokose/data/BS_FM_0-6GHz.txt

RESOURCES += \
    parameters.qrc
