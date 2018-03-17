#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <hackrf.h>
#include <fftw3.h>
#include "qcustomplot.h"
#include "freqsetting.h"
#include "rfusetting.h"

#define MHz 1000000
#define GHz 1000000000


// Definiton of default radio parameters
struct radio_config
{
    hackrf_device *radioID;
    uint32_t LNAgain = 0;
    uint32_t VGAgain = 0;
    uint8_t antPower = false;
    uint64_t rxFreq = 10*MHz;
    double sampleRate = 2*MHz;
    int fftlen = 1024;
    int filterShape = 0;        // 0 - Square, 1 - Hamming, 2 - Hann
    bool hackrf_connected = false;
    int pathFilter = 0; // 0 - bypass, 1 - LP, 2 - HP
} ;


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    hackrf_device* sdr;


private slots:
    void doFFT();
    void on_PBsetFFTLength_clicked();
    void on_PBstartRX_clicked();
    void on_PBstopRX_clicked();
    void on_PBSampleRate_clicked();
    void on_PBExit_clicked();
    void on_PBConnect_clicked();
    void on_PBfftSettings_clicked();
    void on_PBsetFreq_clicked();
    void on_PBsetWinShape_clicked();

    void on_LEfreq_returnPressed();
    void on_LEfreq_textChanged(const QString &arg1);

    void on_SBupperRange_valueChanged(int arg1);


    void on_PBrfuSetting_clicked();

private:
    void defineWindow(double[], int );
    Ui::MainWindow *ui;
    freqSetting *freqWindow;
    RFUsetting *rfuWindow;
    void plot(double dataY[], double dataX[], int N, int graphID, bool switchOrder);


public:
    static fftwf_complex x[1024];
    QTimer guiRefresh;
    //!!! This is bad. this doesn't have to be atomic!!
    static volatile int data_ready;


};

#endif // MAINWINDOW_H
