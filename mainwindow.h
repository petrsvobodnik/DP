#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <hackrf.h>
#include <fftw3.h>
#include "qcustomplot.h"
#include "freqsetting.h"
#include "rfusetting.h"
#include <srsgui/srsgui.h>

#define MHz 1000000
#define GHz 1000000000


// Definiton of default radio parameters
struct radio_config
{
    hackrf_device *radioID; // pointer to radio
    uint32_t LNAgain = 0;   // hack's internal amplifier
    uint32_t VGAgain = 0;   // hack's internal amplifier
    uint8_t antPower = false;
    uint64_t rxFreq = 10*MHz;   // default central frequency [Hz]
    double sampleRate = 2*MHz;  // default sample rate
    int filterShape = 0;        // default FFT filter selector     0 - Square, 1 - Hamming, 2 - Hann
    bool hackrf_connected = false;
    int pathFilter = 0; // hack's internal filter          0 - bypass, 1 - LP, 2 - HP
    float RFUgain = 0;    
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


private slots:
    void on_PBstartRX_clicked();
    void on_PBstopRX_clicked();
    void on_PBSampleRate_clicked();
    void on_PBExit_clicked();
    void on_PBConnect_clicked();
    void on_PBfftSettings_clicked();
    void on_PBsetFreq_clicked();
    void on_PBsetWinShape_clicked();
    void on_PBrfuSetting_clicked();
    void on_SBupperRange_valueChanged(int arg1);

    void on_LEfreq_returnPressed();
    void on_LEfreq_textChanged(const QString &arg1);

    void doFFT();
    void on_PBchooseDir_clicked();
    void on_PBassignFileName_clicked();
    void on_PBsaveStart_clicked();
    void on_PBsaveStop_clicked();
    void on_LEfileName_textChanged(const QString &arg1);
    void on_PBzeroSpan_clicked();


    void on_PBloadData_clicked();

    void on_SliderColorScale_valueChanged(int value);

private:
    void defineWindow(double[], int );
    void plot(double dataY[], double dataX[], int N, int graphID, bool switchOrder);
    void saveMeasuredData(double FFTdata[]);
    void saveMeasInfo(double []);
    void setUpNewFile();
    void setWaterfallData();

    Ui::MainWindow *ui;
    freqSetting *freqWindow;
    RFUsetting *rfuWindow;



public:
    QTimer guiRefresh;
    //!!! This is bad. this doesn't have to be atomic!!
    static volatile int data_ready;


};

#endif // MAINWINDOW_H
