#include "mainwindow.h"
#include <QApplication>
#include <hackrf.h>
#include <fftw3.h>
#include <iostream>

#define MHz 1000000
#define GHz 1000000000

struct radio_config
{
    hackrf_device *radio;
    uint32_t LNAgain;
    uint32_t VGAgain;
    const uint8_t antPower = false;
    const uint64_t rx_freq_hz = 20*MHz;
    const double sampleRate_hz = 10*MHz;

} hackConfig;

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
