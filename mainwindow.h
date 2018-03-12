#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <hackrf.h>
#include <fftw3.h>
#include "freqsetting.h"
#include "qcustomplot.h"

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

    void on_LEfreq_returnPressed();
    void on_LEfreq_textChanged(const QString &arg1);

    void on_SBupperRange_valueChanged(int arg1);


private:
    void defineWindow(double[], int );
    Ui::MainWindow *ui;
    bool hackrf_connected;
    double bw;
    freqSetting *freqWindow;
    void plot(double dataY[], double dataX[], int N, int graphID, bool switchOrder);


public:
    static fftwf_complex x[1024];
    QTimer guiRefresh;
    //!!! This is bad. this doesn't have to be atomic!!
    static volatile int data_ready;


};

#endif // MAINWINDOW_H
