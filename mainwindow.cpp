#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <fftw3.h>
#include "freqsetting.h"
#include "qcustomplot.h"

int data_cb(hackrf_transfer*);
const int N = 1024;
//int *pocetVzorku;
//static fftw_complex x[N];
//static fftw_complex x[1024];

volatile int MainWindow::data_ready = 0;
fftw_complex MainWindow::x[N];
//static fftw_complex x[N];
double spectrum [N];
double vzorky [N];
double fftFiltr [N];

//!!! This is bad. this doesn't have to be atomic!!
volatile int data_ready = 0;
#define REAL 0
#define IMAG 1

// zkusit vykreslit jen okno do grafu soucasne
// pokoumat nasobeni

// GUI konstruktor
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    hackrf_init();
    this->hackrf_connected = false;

    ui->setupUi(this);


    ui->PBExit->setText("Close");

    QStringList listRates, listLength, listWindow;
    listRates << "2" << "4" << "8" << "10" << "20";
    ui->CBsampleRate->addItems(listRates);

    listLength << "1024"<<"2048"<<"4096"<<"8192"<<"16384";
    ui->CBwinLen->addItems(listLength);

    listWindow << "Square" << "Hamming" << "Hann";
    ui->CBwinShape->addItems(listWindow);

    ui->PBstartRX->setDisabled(true);
    ui->PBstopRX->setDisabled(true);
    ui->PBSampleRate->setDisabled(true);
    ui->PBsetFFTLength->setDisabled(true);

    ui->fftGraph->addGraph();
    defineWindow(fftFiltr,ui->CBwinShape->currentIndex());

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


void MainWindow::on_PBConnect_clicked()
{
    hackrf_device_list_t* deviceList = hackrf_device_list();
    const char *SN = *deviceList->serial_numbers;


    if(!this->hackrf_connected){
        if(hackrf_open_by_serial(SN, &sdr)!= HACKRF_SUCCESS) // rádio se otevírá podle SN
            puts("Error opening device");
        else{
            std::cout << "connected to: "<< SN << std::endl;
            ui->PBstartRX->setDisabled(false);
            ui->PBstopRX->setDisabled(false);
            ui->PBSampleRate->setDisabled(false);
            ui->PBsetFFTLength->setDisabled(false);
        }

        const uint32_t freq = 100000000;
        hackrf_set_freq(sdr, freq);

        hackrf_set_amp_enable(sdr,'0');
        this->hackrf_connected = true;
        MainWindow::data_ready = 0;
    }
}

void MainWindow::on_PBsetFFTLength_clicked()
{
    double delka= ui->CBwinLen->currentText().toInt();
    std::cout << delka << std::endl;
    hackrf_set_sample_rate(sdr, bw);

    int typFiltru = ui->CBwinShape->currentIndex();
    defineWindow(fftFiltr, typFiltru);
}


void MainWindow::on_PBExit_clicked()
{
    window()->close();
}

void MainWindow::on_PBfftSettings_clicked()
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

void MainWindow::on_PBSampleRate_clicked()
{
    bw = ui->CBsampleRate->currentText().toDouble() * 1000000;
    std::cout << double(bw) << std::endl;
}

int data_cb(hackrf_transfer* trn){
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
    ui->fftGraph->yAxis->setRange(0,60);
//    ui->graf->yAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->fftGraph->replot();
//    ui->fftGraph->update();
    return;
}


void MainWindow::doFFT(){
    static int a = 0;
    QString s = QString::number(a);

    fftw_complex y[N];

    while(!MainWindow::data_ready);

//    // FFT window shaping
//    MainWindow::x[REAL] = MainWindow::x[REAL][]*fftFiltr[1][];
//    MainWindow::x[IMAG] = MainWindow::x[IMAG]*fftFiltr;
//    MainWindow::x[][0] *= fftFiltr[];

    for (int i=0; i<N; i++){
        MainWindow::x[i][0] *= fftFiltr[i];
        MainWindow::x[i][1] *= fftFiltr[i];
    }

    // Planning the fft
    fftw_plan plan = fftw_plan_dft_1d(N,MainWindow::x,y,FFTW_FORWARD,FFTW_ESTIMATE);
    fftw_execute(plan);
    for (int i=0; i<N; i++){
        spectrum[i] = 10*log10(sqrt(pow(y[i][REAL],2) + pow(y[i][IMAG],2)));
        vzorky[i] = i;
    }

    plot(spectrum, vzorky, N);


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

void MainWindow::defineWindow(double fftWindow[], int typ){
    switch (typ) {
    case 0: // Rectangle
        for (int i=0; i<N; i++)
            fftWindow[i]=1;
                    break;

    case 1: // Hamming
        for (int i=0; i<N; i++)
            fftWindow[i] = 0.54 - 0.46*cos((2*M_PI*i)/(N-1));
        break;

    case 2: // Hann
        for (int i=0; i<N; i++)
            fftWindow[i]= pow(sin((M_PI*i)/(N-1)), 2);
        break;

    default:
        break;
    }
}
