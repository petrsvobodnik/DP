#include "mainwindow.h"
#include <QApplication>
#include <hackrf.h>
#include <fftw3.h>
#include <iostream>

#define MHz 1000000
#define GHz 1000000000



hackrf_device *radio;

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;

    w.show();

    int ret = a.exec();
    return ret;
    //return a.exec();
//    return 0;
}
