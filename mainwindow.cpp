#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <fftw3.h>
#include "freqsetting.h"
#include "qcustomplot.h"
#include "main.h"

radio_config hackConfig;

int data_cb(hackrf_transfer*);
const int N = 1024;

//static fftwf_complex x[N];
//static fftwf_complex x[1024];

volatile int MainWindow::data_ready = 0;
fftwf_complex MainWindow::x[N];
//static fftwf_complex x[N];
double spectrum [N];
double spectrum1 [N];
double samples [N];
double fftFiltr [N];
//radio_config MainWindow::hackConfig;

//!!! This is bad. this doesn't have to be atomic!!
volatile int data_ready = 0;
#define REAL 0
#define IMAG 1


// GUI constructor
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    hackrf_init();
    hackrf_device_list_t* deviceList = hackrf_device_list();
    hackConfig.hackrf_connected = false;

    ui->setupUi(this);

    ui->labelNoofDevices->setText("HackRF's found: "+QString::number(deviceList->devicecount));


    // Definition of CB's text items
    QStringList listRates, listLength, listWindow, listFreqChoice, listSN;

    for (int i =0; i<=deviceList->devicecount-1; i++){
        listSN << deviceList->serial_numbers[i]+16;
    }
    ui->CBhackSN->addItems(listSN);

    listRates << "2" << "4" << "8" << "10" << "20";
    ui->CBsampleRate->addItems(listRates);

    listLength << "1024"<<"2048"<<"4096"<<"8192"<<"16384";
    ui->CBwinLen->addItems(listLength);

    listWindow << "Square" << "Hamming" << "Hann";
    ui->CBwinShape->addItems(listWindow);

    listFreqChoice << "Custom" << "Preset 1" << "Preset 2" << "Preset 3";
    ui->CBfreq->addItems(listFreqChoice);

    ui->LEfreq->setText("10000");
    ui->LEfreq->setAlignment(Qt::AlignRight);


    // setting buttons to be inactive till Hack is connected
    ui->PBstartRX->setDisabled(true);
    ui->PBstopRX->setDisabled(true);
    ui->PBSampleRate->setDisabled(true);
    ui->PBsetFFTLength->setDisabled(true);
    ui->PBsetFreq->setDisabled(true);


    // graph constructors
    ui->fftGraph->addGraph();
    ui->fftGraph->graph(0)->setPen(QPen(Qt::blue));
    ui->fftGraph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));
    ui->fftGraph->graph(0)->setName("FFT diagram");

    ui->fftGraph->addGraph(ui->fftGraph->xAxis, ui->fftGraph->yAxis2);
    ui->fftGraph->graph(1)->setPen(QPen(Qt::red));
    ui->fftGraph->graph(1)->setName("Windowing characteristic");

    ui->fftGraph->xAxis->setRange(0,ui->CBwinLen->currentData().toInt());
    ui->fftGraph->yAxis->setRange(0,50);
    ui->fftGraph->yAxis2->setRange(0,1);
    ui->fftGraph->yAxis2->setVisible(true);
    ui->fftGraph->legend->setVisible(true);

    ui->fftGraph->setInteractions(QCP::iRangeZoom | QCP::iSelectPlottables| QCP::iRangeDrag);
    ui->fftGraph->axisRect()->setRangeDrag(Qt::Horizontal);
    ui->fftGraph->axisRect()->setRangeZoom(Qt::Horizontal);

    // Y-axis range changing
    ui->SBupperRange->setValue(ui->fftGraph->yAxis->range().upper);
    ui->SBupperRange->setDisabled(true);
    ui->labelDate->hide();
    ui->labelTime->hide();

    // Preparation for adding another graph
//    ui->fftGraph->addGraph();
//    ui->fftGraph->graph(2)->setPen(QPen(Qt::green));

    defineWindow(fftFiltr,ui->CBwinShape->currentIndex());

    connect(&guiRefresh, SIGNAL(timeout()),this, SLOT(doFFT()));
}

// GUI destructor
MainWindow::~MainWindow()
{
//   Disconnecting from radio
    if(hackConfig.hackrf_connected){
        hackrf_stop_rx(sdr);
        hackrf_close(sdr);
        puts("Disconnected");
    }
    hackrf_exit();

    delete ui;
}


void MainWindow::on_PBConnect_clicked()
{
    // loading list of connected devices
    hackrf_device_list_t* deviceList = hackrf_device_list();
    const char *SN = *deviceList->serial_numbers;
//    char hilfe = ui->CBhackSN->currentData().toChar()
//    const char *SN = hilfe;

    if(!hackConfig.hackrf_connected){
        if(hackrf_open_by_serial(SN, &sdr)!= HACKRF_SUCCESS) // radio is being identified by its SN
        {
            QMessageBox::warning(this, "Not connected!", "No HackRF device found");
            puts("Error opening device");
        }
        else{
            // activation of GUI buttons
            hackConfig.radioID = sdr;
            std::cout << "connected to: "<< SN+16 << std::endl;
            ui->PBstartRX->setDisabled(false);
            ui->PBstopRX->setDisabled(false);
            ui->PBSampleRate->setDisabled(false);
            ui->PBsetFFTLength->setDisabled(false);
            ui->PBsetFreq->setDisabled(false);
            ui->SBupperRange->setDisabled(false);
            ui->PBConnect->setText("Connected");
            ui->statusBar->showMessage("Connected");
        }

        hackConfig.hackrf_connected = true;
        MainWindow::data_ready = 0;
    }
}

void MainWindow::on_PBsetFFTLength_clicked()
{
    double len= ui->CBwinLen->currentText().toInt();
    hackConfig.fftlen = len;
}

void MainWindow::on_PBExit_clicked()
{
    window()->close();
}

void MainWindow::on_PBfftSettings_clicked()
{
    // Displaying FreqSetting window
    freqWindow = new freqSetting(this);
    freqWindow->show();
    freqWindow->setRadio(sdr);
}

void MainWindow::on_PBstartRX_clicked()
{
    // Start reception
//    hackrf_start_rx(sdr, data_cb, NULL);
    hackrf_start_rx(hackConfig.radioID, data_cb, NULL);
    guiRefresh.start(500);
}

void MainWindow::on_PBstopRX_clicked()
{
    // End reception
//    hackrf_stop_rx(sdr);
    hackrf_stop_rx(hackConfig.radioID);
    guiRefresh.stop();
}

void MainWindow::on_PBSampleRate_clicked()
{
    // change of sample rate
//    bw = ui->CBsampleRate->currentText().toDouble() * 1000000;
    hackConfig.sampleRate = ui->CBsampleRate->currentText().toDouble() * 1000000;

    hackrf_set_sample_rate(sdr, hackConfig.sampleRate);
}

void MainWindow::on_LEfreq_returnPressed()
{
    // Set new frequency by pressing Enter
//    const uint64_t frequency= ui->LEfreq->text().toUInt()*1000;
//    hackrf_set_freq(sdr, frequency);
    hackConfig.rxFreq = ui->LEfreq->text().toUInt()*1000;
    hackrf_set_freq(sdr, hackConfig.rxFreq);
}

void MainWindow::on_PBsetFreq_clicked()
{
   on_LEfreq_returnPressed();
}

void MainWindow::on_LEfreq_textChanged(const QString &arg1)
{
    // Treating frequency out of range values
//    QPalette LEcolor; // left here only for demonstration of usage

    if ((arg1.toInt()<1000) | (arg1.toInt()>6000000)){
        ui->LEfreq->setStyleSheet("QLineEdit {background-color: rgb(255, 117, 117);}");
        ui->PBsetFreq->setDisabled(true);
        ui->label_5->setText("Frequency has to be in range of 1MHz - 6GHz");
        ui->label_5->show();
//        LEcolor.setBrush(QPalette::Base, rgb(255, 117, 117));
    }
    else
    {
        ui->LEfreq->setStyleSheet("QLineEdit {background-color: white;}");
        //        LEcolor.setBrush(QPalette::Base, Qt::white);
        ui->PBsetFreq->setDisabled(false);
        ui->label_5->hide();
    }

//    ui->LEfreq->setPalette(LEcolor);
}

void MainWindow::on_SBupperRange_valueChanged(int arg1)
{
    ui->fftGraph->yAxis->setRangeUpper(arg1);
}

int data_cb(hackrf_transfer* trn){
    // callback function for data reception from Hack
    if (!MainWindow::data_ready)
    {
        int8_t re, im;
        for (int i=0; i<N; i++){
            re = trn->buffer[i*2];
            im = trn->buffer[i*2+1];

            MainWindow::x[i][REAL] = (double)re; //((trn->buffer[i*2])-128);re // MainWindow::x
            MainWindow::x[i][IMAG] = (double)im; //double((trn->buffer[i*2+1])-128);
        }
        MainWindow::data_ready=1;
    }
    return 0;
}

void MainWindow::plot(double dataY[], double dataX[], int N, int graphID, bool switchOrder){
    // function for plotting data. Input arrays are converted to vectors. Optionally, second half of data is moved to the front to center the tuned frequency
    std::vector<double> x1;
    std::vector<double> y1;

    y1.assign(dataY, dataY+N);


    if (switchOrder){       // switchOrder serves to push the second half of FFT results to front --> tuned frequency is in the center of plot
        std::vector<double> x0;
        x1.assign(dataX+N/2, dataX+N);
        x0.assign(dataX, dataX+N/2);
        x1.insert(x1.end(), x0.begin(), x0.end());
    }
    else
        x1.assign(dataX, dataX+N);


    QVector<double> x = QVector<double>::fromStdVector(x1);
    QVector<double> y = QVector<double>::fromStdVector(y1);


    ui->fftGraph->graph(graphID)->setData(x,y);
    ui->fftGraph->graph(graphID)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->fftGraph->graph(graphID)->setLineStyle(QCPGraph::lsLine);


    ui->fftGraph->xAxis->rescale();
    ui->fftGraph->replot();
//    ui->fftGraph->update();
    return;
}

void MainWindow::doFFT(){
    static int a = 0;
    QString s = QString::number(a);

    fftwf_complex y[N];

    while(!MainWindow::data_ready);

//    // FFT window shaping
    for (int i=0; i<N; i++){
        MainWindow::x[i][0] *= fftFiltr[i];
        MainWindow::x[i][1] *= fftFiltr[i];
    }

    // Planning the fft
    fftwf_plan plan = fftwf_plan_dft_1d(N,MainWindow::x,y,FFTW_FORWARD,FFTW_ESTIMATE);
    fftwf_execute(plan);
    for (int i=0; i<N; i++){
        spectrum[i] = 10*log10(sqrt(pow(y[i][REAL],2) + pow(y[i][IMAG],2)));
//        spectrum[i] = (MainWindow::x[i][REAL]);   // plot data in time domain (real component of the signal)
        samples[i] = i;
//        spectrum1[i] = (MainWindow::x[i][IMAG]);  // plot data in time domain (imag component of the signal)
    }


// // plotting graphs
    plot(fftFiltr, samples, N, 1, false);
    plot(spectrum, samples, N, 0, true);
//    plot(spectrum1, samples, N, 2);

//    //cleaning
//    fftwf_destroy_plan(plan);
//    fftwf_cleanup();

    MainWindow::data_ready = 0;

    a++;
    s.asprintf("%d",a);
    ui->label_2->setText(s);
    ui->labelTime->setText("Current time: "+QDateTime::currentDateTime().toString("hh:mm:ss")); //dd.MM.yy"));
    ui->labelTime->show();
    ui->labelDate->setText("Current date: "+QDateTime::currentDateTime().toString("dd.MM.yy"));
    ui->labelDate->show();
}

void MainWindow::defineWindow(double fftWindow[], int typ){
    // calculation of windows
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


void MainWindow::on_PBsetWinShape_clicked()
{
    //    int filterType = ui->CBwinShape->currentIndex();
    hackConfig.filterShape = ui->CBwinShape->currentIndex();
    defineWindow(fftFiltr, hackConfig.filterShape);
}
