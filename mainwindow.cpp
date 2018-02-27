#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <fftw3.h>
#include "freqsetting.h"
#include "qcustomplot.h"


const int N = 1024;
//int *pocetVzorku;
//static fftw_complex x[N];
//static fftw_complex x[1024];

volatile int MainWindow::data_ready = 0;
fftw_complex MainWindow::x[N];
//static fftw_complex x[N];
double spectrum [N];
double vzorky [N];

//!!! This is bad. this doesn't have to be atomic!!
volatile int data_ready = 0;
#define REAL 0
#define IMAG 1

// trn->buffer - proměnná, která obsahuje prijata data
int data_cb(hackrf_transfer* trn){      // callback provadi dalsi ukoly
//    printf("\nPrijato %d dat", trn->valid_length);
    if (!MainWindow::data_ready)
    {
        for (int i=0; i<N; i++){
            MainWindow::x[i][REAL] = trn->buffer[i*2];
            MainWindow::x[i][IMAG] = trn->buffer[i*2+1];
        }
        MainWindow::data_ready=1;
    }
    return 0;
}

// GUI konstruktor
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    hackrf_init();
    this->hackrf_connected = false;

    ui->setupUi(this);

    ui->ExitButton->setText("Close");

    QStringList fsRates;
    fsRates << "2" << "4" << "8" << "10" << "20";
    ui->fsComboBox->addItems(fsRates);

    QStringList fftValues;
    fftValues << "1024"<<"2048"<<"4096"<<"8192"<<"16384";
    ui->fftLength->addItems(fftValues);

    ui->fftGraph->addGraph();

    connect(&guiRefresh, SIGNAL(timeout()),this, SLOT(doFFT()));
}

// GUI destruktor
MainWindow::~MainWindow()
{
//    if(this->hackrf_connected){
    if(hackrf_connected){
        hackrf_stop_rx(sdr);
        hackrf_close(sdr);
        puts("Disconnected");
    }
    hackrf_exit();

    delete ui;
}


void MainWindow::doFFT(){
    static int a = 0;
    QString s = QString::number(a);

    fftw_complex y[1024];

    while(!MainWindow::data_ready);

    // Planning the fft
    fftw_plan plan = fftw_plan_dft_1d(1024,MainWindow::x,y,FFTW_FORWARD,FFTW_ESTIMATE);
    fftw_execute(plan);
    for (int i=0; i<1024; i++){
        spectrum[i] = 10*log10(sqrt(pow(y[i][REAL],2) + pow(y[i][IMAG],2)));
        vzorky[i] = i;
    }

    plot(spectrum, vzorky, 1024);


//    //cleaning
//    fftw_destroy_plan(plan);
//    fftw_cleanup();

    MainWindow::data_ready = 0;

    a++;
    s.asprintf("%d",a);
    ui->label_2->setText(s);
    for (int i=0; i<10; i++)
        std::cout << spectrum[i] << " ";
    std::cout << std::endl;
}

void MainWindow::on_btnConnect_clicked(){
    hackrf_device_list_t* deviceList = hackrf_device_list();
    const char *SN = *deviceList->serial_numbers;

    const uint32_t freq = 100000000;

    if(!this->hackrf_connected){
        if(hackrf_open_by_serial(SN, &sdr)!= HACKRF_SUCCESS) // rádio se otevírá podle SN
            puts("Error opening device");
        else{
            std::cout << "connected to: "<< SN << std::endl;
        }

        hackrf_set_freq(sdr, freq);
//        hackrf_set_sample_rate(sdr, bandwidth);
        hackrf_set_amp_enable(sdr,'0');
        this->hackrf_connected = true;
        MainWindow::data_ready = 0;
    }
}

void MainWindow::on_setFFTLength_clicked()
{
    double delka= ui->fftLength->currentText().toInt();
    std::cout << delka << std::endl;
    hackrf_set_sample_rate(sdr, bw);
}

void MainWindow::on_SampleRateButton_clicked()
{
    bw = ui->fsComboBox->currentText().toDouble() * 1000000;
    std::cout << double(bw) << std::endl;
}

void MainWindow::on_ExitButton_clicked()
{
    window()->close();
}

void MainWindow::on_fftSettingsButton_clicked()
{
    freqWindow = new freqSetting(this);
    freqWindow->show();
    freqWindow->setRadio(sdr);
}

void MainWindow::on_PBstartRX_clicked()
{
    // Zahájení příjmu
    hackrf_start_rx(sdr, data_cb, NULL);

    guiRefresh.start(500);
}

void MainWindow::on_PBstopRX_clicked()
{
    hackrf_stop_rx(sdr);
    guiRefresh.stop();
}


void MainWindow::plot(double dataY[], double dataX[], int N){
    std::vector<double> x1;
    std::vector<double> y1;

    y1.assign(dataY, dataY+N);
    x1.assign(dataX, dataX+N);

    QVector<double> x = QVector<double>::fromStdVector(x1);
    QVector<double> y = QVector<double>::fromStdVector(y1);


    ui->fftGraph->graph(0)->setData(x,y);
    ui->fftGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->fftGraph->graph(0)->setLineStyle(QCPGraph::lsLine);

    ui->fftGraph->xAxis->setRange(0,N-1);
    ui->fftGraph->yAxis->setRange(0,100);
//    ui->graf->yAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->fftGraph->replot();
    return;
}
