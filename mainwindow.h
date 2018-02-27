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
    void on_btnConnect_clicked();
    void on_setFFTLength_clicked();
    void on_ExitButton_clicked();
    void on_SampleRateButton_clicked();
    void on_fftSettingsButton_clicked();

    void on_PBstartRX_clicked();
    void on_PBstopRX_clicked();

private:
    Ui::MainWindow *ui;
    bool hackrf_connected;
    double bw;
    freqSetting *freqWindow;
    void plot(double dataY[], double dataX[], int N);


public:
    static fftw_complex x[1024];
    QTimer guiRefresh;
    //!!! This is bad. this doesn't have to be atomic!!
    static volatile int data_ready;


};

#endif // MAINWINDOW_H
