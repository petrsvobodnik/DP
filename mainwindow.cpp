#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <fftw3.h>
#include "freqsetting.h"
#include "qcustomplot.h"
#include "zerospan.h"
#include "loaddata.h"

int data_cb(hackrf_transfer*);

radio_config hackConfig;
const int N = 1024;
volatile int MainWindow::data_ready = 0;
double spectrum [N];
double samples [N];
double fftFiltr [N];
float xf [N][2];
QString saveFileName;
bool saveEnabled = 0;
int saveCounter;
QTime midnight(23, 59, 59, 0);
int WFlen = 100;
int freqUnits;

QCPColorMap *colorMap = NULL;

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

    if (deviceList->devicecount == 0)
        ui->labelNoofDevices->setText("No HackRF connected");
    else
        ui->labelNoofDevices->setText("HackRF's found: "+QString::number(deviceList->devicecount));


    // Definition of CB's text items
    QStringList listRates, listWindow, listFreqChoice, listSN, listUnits;

    for (int i =0; i<=deviceList->devicecount-1; i++){
        listSN << deviceList->serial_numbers[i]+16;
    }
    ui->CBhackSN->addItems(listSN);

    listRates << "2" << "4" << "8" << "10" << "20";
    ui->CBsampleRate->addItems(listRates);
    ui->CBsampleRate->setCurrentIndex(3);  // setting 10MHz as standard sample rate
    ui->CBsampleRate->setDisabled(true);

    listWindow << "Square" << "Hamming" << "Hann";
    ui->CBwinShape->addItems(listWindow);

    listFreqChoice << "Custom" << "Preset 1" << "Preset 2" << "Preset 3";
    ui->CBfreq->addItems(listFreqChoice);

    listUnits << "Hz" << "kHz" << "MHz" << "GHz";
    ui->CBunits->addItems(listUnits);
    ui->CBunits->setCurrentIndex(2);


    ui->LEfreq->setText(QString::number(double(hackConfig.rxFreq)/freqUnits));
    ui->LEfreq->setAlignment(Qt::AlignRight);


    // setting buttons to be inactive till Hack is connected
    ui->PBstartRX->setDisabled(true);
    ui->PBstopRX->setDisabled(true);
    ui->PBsetFreq->setDisabled(true);
    ui->PBsaveStart->setDisabled(true);


    // graph constructors
    ui->fftGraph->addGraph();
    ui->fftGraph->graph(0)->setPen(QPen(Qt::blue));
    ui->fftGraph->graph(0)->setName("FFT diagram");

    ui->fftGraph->addGraph(ui->fftGraph->xAxis, ui->fftGraph->yAxis2);
    ui->fftGraph->graph(1)->setPen(QPen(Qt::red));
    ui->fftGraph->graph(1)->setName("Windowing characteristic");

    ui->fftGraph->yAxis->setRange(-100,-20);
    ui->fftGraph->yAxis->setLabel("Power [dBm]");
    ui->fftGraph->yAxis2->setRange(0,1);
    ui->fftGraph->yAxis2->setVisible(false);
    ui->fftGraph->xAxis->setLabel("Frequency [" + ui->CBunits->currentText() + "]");
    ui->fftGraph->legend->setVisible(true);

    ui->fftGraph->setInteractions(QCP::iRangeZoom | QCP::iSelectPlottables| QCP::iRangeDrag);
    ui->fftGraph->axisRect()->setRangeDrag(Qt::Horizontal);
    ui->fftGraph->axisRect()->setRangeZoom(Qt::Horizontal);

    // Y-axis range changing
    ui->SBupperRange->setValue(ui->fftGraph->yAxis->range().upper);
    ui->SBupperRange->setDisabled(true);

    ui->labelDate->hide();
    ui->labelTime->hide();
    ui->labelSaving->hide();
    ui->labelFileExists->hide();

    ui->LEfileName->setText(QDir::homePath());
    ui->labelFileExists->hide();
    ui->PBsaveStart->setDisabled(true);
    ui->PBsaveStop->setDisabled(true);

    ui->label_5->hide();
    ui->LEfreq->setStyleSheet("QLineEdit {background-color: white;}");
    ui->LEfreq->setDisabled(true);


//    // Preparation for adding another graph
//    ui->fftGraph->addGraph();
//    ui->fftGraph->graph(2)->setPen(QPen(Qt::green));

    // waterfall initialization
    colorMap = new QCPColorMap(ui->waterfall->xAxis, ui->waterfall->yAxis);
    colorMap->data()->setSize(1024, WFlen);

//    QCPColorScale *colorScale = new QCPColorScale(ui->waterfall);
//    ui->waterfall->plotLayout()->addElement(0, 1, colorScale);
    ui->waterfall->yAxis->setTicks(false);
//    colorScale->setType(QCPAxis::atRight);
//    colorMap->setColorScale(colorScale);
//    colorScale->setGradient(QCPColorGradient::gpSpectrum);
    colorMap->setGradient(QCPColorGradient::gpSpectrum);
    colorMap->setDataRange(QCPRange(-100, -60));
    ui->waterfall->xAxis->setLabel("Frequency [" + ui->CBunits->currentText() + "]");

    ui->GBsaving->setDisabled(true);



    this->setMinimumSize(900, 720);

    defineWindow(fftFiltr,ui->CBwinShape->currentIndex());

    connect(&guiRefresh, SIGNAL(timeout()),this, SLOT(doFFT()));
//    connect(&unitSelect, SIGNAL(buttonClicked(int)), this, SLOT(on_CBunits_currentIndexChanged(int)));
//    connect(this, this->resizeEvent();)
}

// GUI destructor
MainWindow::~MainWindow()
{
//   Disconnecting from radio
    if(hackConfig.hackrf_connected){
        hackrf_stop_rx(hackConfig.radioID);
        hackrf_close(hackConfig.radioID);
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

    if(!hackConfig.hackrf_connected){
        if(hackrf_open_by_serial(SN, &hackConfig.radioID)!= HACKRF_SUCCESS) // radio is being identified by its SN
            QMessageBox::warning(this, "Not connected!", "No HackRF device found");

        else{
            // activation of GUI buttons
            std::cout << "connected to: "<< SN+16 << std::endl;
            ui->PBstartRX->setDisabled(false);
            ui->PBsetFreq->setDisabled(false);
            ui->SBupperRange->setDisabled(false);
            ui->PBConnect->setText("Connected");
            ui->actionStart_saving->setDisabled(1);
            ui->actionStop_saving->setDisabled(1);
//            ui->PBsaveStart->setDisabled(false);
            ui->LEfreq->setDisabled(false);
            ui->CBsampleRate->setDisabled(false);
            ui->GBsaving->setDisabled(false);

            ui->statusBar->showMessage("Connected");

            // making sure to set zero gains of Hack's amplifiers
            hackrf_set_vga_gain(hackConfig.radioID, 0);
            hackrf_set_lna_gain(hackConfig.radioID, 0);
        }

        hackConfig.hackrf_connected = true;
        MainWindow::data_ready = 0;
    }
}

void MainWindow::on_PBExit_clicked()
{
    if (saveEnabled)
        on_PBsaveStop_clicked();
    on_PBstopRX_clicked();
    window()->close();
}

void MainWindow::on_PBfftSettings_clicked()
{
    // Displaying FreqSetting window
    freqWindow = new freqSetting(this);
    freqWindow->show();
    ui->fftGraph->yAxis2->setVisible(true);
}

void MainWindow::on_PBstartRX_clicked()
{
    // Start reception
    hackrf_start_rx(hackConfig.radioID, data_cb, NULL);
    guiRefresh.start(500);
    ui->PBstopRX->setDisabled(false);
}

void MainWindow::on_PBstopRX_clicked()
{
    // End reception
    hackrf_stop_rx(hackConfig.radioID);
    guiRefresh.stop();
}

void MainWindow::on_LEfreq_returnPressed()
{
    // Set new frequency by pressing Enter
    ui->LEfreq->setStyleSheet("QLineEdit {background-color: white;}");
    ui->label_5->hide();

    if (ui->LEfreq->text().isEmpty()){  // checking whether user entered a value
        ui->label_5->setText("No value set");
        ui->label_5->show();
    }
    else{
        hackConfig.rxFreq = ui->LEfreq->text().toUInt()*freqUnits;
        hackrf_set_freq(hackConfig.radioID, hackConfig.rxFreq);
    }
}

void MainWindow::on_PBsetFreq_clicked()
{
   on_LEfreq_returnPressed();
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

//            MainWindow::x[i][REAL] = (double)re; //((trn->buffer[i*2])-128);re // MainWindow::x
//            MainWindow::x[i][IMAG] = (double)im; //double((trn->buffer[i*2+1])-128);

            xf[i][REAL] = (double)re;
            xf[i][IMAG] = (double)im;

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
    return;
}

void MainWindow::doFFT(){
    static int a = 0;
    QString s = QString::number(a);

    fftwf_complex y[N];

    while(!MainWindow::data_ready);

//    // FFT window shaping
    for (int i=0; i<N; i++){
        xf[i][0] *= fftFiltr[i];
        xf[i][1] *= fftFiltr[i];
    }

    // Planning the fft
    fftwf_plan FFTplan = fftwf_plan_dft_1d(N,xf,y,FFTW_FORWARD,FFTW_ESTIMATE);
    fftwf_execute(FFTplan);
    for (int i=0; i<N; i++){
        spectrum[i] = 5*log10(y[i][REAL]*y[i][REAL] + y[i][IMAG]*y[i][IMAG] ) - 100; // computing w/o sqrt and pow
//        spectrum[i] = (MainWindow::x[i][REAL]);   // plot data in time domain (real component of the signal)
//        spectrum1[i] = (MainWindow::x[i][IMAG]);  // plot data in time domain (imag component of the signal)

        samples[i] = (hackConfig.rxFreq - hackConfig.sampleRate/2 + (hackConfig.sampleRate/N)*i)/freqUnits;  // Assign frequency to FFT data
    }


// // plotting graphs
    plot(fftFiltr, samples, N, 1, false);
    plot(spectrum, samples, N, 0, true);
//    plot(spectrum1, samples, N, 2, true);

    setWaterfallData();

// // Saving data
    if (saveEnabled){
        if  (QTime::currentTime()>midnight){    // at midnight open up new file
            setUpNewFile();
        }
        else
            saveMeasuredData(spectrum);
    }

//    //cleaning
//    fftwf_destroy_plan(plan);
//    fftwf_cleanup();

    MainWindow::data_ready = 0;

    a++;
    s.asprintf("%d",a);
    ui->label_2->setText(s);
    ui->labelTime->setText("Current time: "+QDateTime::currentDateTime().toString("hh:mm:ss"));
    ui->labelDate->setText("Current date: "+QDateTime::currentDateTime().toString("dd.MM.yy"));
    ui->labelTime->show();
    ui->labelDate->show();
}

void MainWindow::setWaterfallData()
{


    colorMap->data()->setKeySize(N);
    colorMap->data()->setKeyRange(QCPRange((hackConfig.rxFreq - hackConfig.sampleRate/2)/freqUnits, (hackConfig.rxFreq + hackConfig.sampleRate/2)/freqUnits));
    colorMap->data()->setValueRange(QCPRange(0, colorMap->data()->valueSize()));

        for (int i=0; i<WFlen; i++){               // rows        // adding new data to the top of diagram
            for (int j=0; j<N; j++){               // columns
                if( i<(WFlen-1))    // shift existing data
                     colorMap->data()->setCell(j, i, colorMap->data()->cell(j, i+1));

                else        // Add new data
                {
//                    ui->SliderScaling ensures range change in waterfall diagram by adding a value
//                    colorMap->data()->setCell(j, WFlen-1, spectrum[j] + ui->SliderScaling->value());  // original code


                    if (j<N/2)     // switching the left and right half of WF diagram, so center frequency is in the middle
                        colorMap->data()->setCell(j+N/2, WFlen-1, spectrum[j] + ui->SliderScaling->value());
                    else
                        colorMap->data()->setCell(j-N/2, WFlen-1, spectrum[j] + ui->SliderScaling->value());
                }

                // Code for shifting WF diagram up (instead of down)
//                    colorMap->data()->setCell(j, WFlen-i, colorMap->data()->cell(j, WFlen-i-1));
//                else
//                    colorMap->data()->setCell(j, 1, spectrum[j]);
            }
        }

    ui->waterfall->rescaleAxes();
    ui->waterfall->replot();


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

void MainWindow::on_PBrfuSetting_clicked()
{
    rfuWindow = new RFUsetting(this);
    rfuWindow->setWindowTitle("Antenna Unit Settings");
    rfuWindow->show();
}

void MainWindow::saveMeasuredData(double FFTdata[]){        // saving data itself
    QFile fileInput(saveFileName);

    if (fileInput.open(QIODevice::WriteOnly| QIODevice::Append)){
        QTextStream textStream (&fileInput);
        textStream.setRealNumberPrecision(4);
        textStream << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "\t";

        for (int i=0; i<1023; i++){     // missing one component as max count of csv columns is 1024!!!
            textStream << FFTdata[i] << "\t";
        }
        textStream<< "\n";
    }
    saveCounter++;
}

void MainWindow::on_PBchooseDir_clicked()
{
    ui->LEfileName->setText(QFileDialog::getExistingDirectory(this, "Choose directory", "/home/golem/Downloads"));
}

void MainWindow::on_PBassignFileName_clicked()
{
    QString name = ui->LEfileName->text();

    if(name.contains("csv"))
       saveFileName = name.left(name.lastIndexOf(QChar('/'))+1) + QDateTime::currentDateTime().toString("dd-MM-yy") + "_" + QString::number( hackConfig.rxFreq/uint64_t (1000000)) + "MHz.csv";
    else
        saveFileName = ui->LEfileName->text() + "/" + QDateTime::currentDateTime().toString("dd-MM-yy") + "_" + QString::number( hackConfig.rxFreq/uint64_t (1000000)) + "MHz.csv";


    ui->LEfileName->setText(saveFileName);
    if(hackConfig.hackrf_connected)     // checking whether radio is connected
        ui->PBsaveStart->setDisabled(false);
    saveCounter = 1;
}

void MainWindow::on_PBsaveStart_clicked()
{
    // Assuring to not overwrite previous data
    if (saveFileName==""){
        QMessageBox::warning(this, "No file selected", "You need to choose file for saving again!");
        return;
    }

    QFile fileInput(saveFileName);

    if (fileInput.exists()){
        QMessageBox::StandardButton reply;
          reply = QMessageBox::question(this, "Warning", "Existing file will be \n overwritten! Continue?", QMessageBox::Yes|QMessageBox::No);
          if (reply == QMessageBox::No)
             return;
    }

    saveMeasInfo(samples);

    ui->LEfreq->setDisabled(true);
    ui->PBsetFreq->setDisabled(true);
    ui->PBsaveStop->setDisabled(false);
    ui->PBsaveStart->setDisabled(true);
    ui->PBassignFileName->setDisabled(true);
    ui->PBchooseDir->setDisabled(true);
    ui->LEfileName->setReadOnly(true);
    ui->labelSaving->show();

    saveEnabled = true;     // allows saving in doFFT() function
}

void MainWindow::setUpNewFile(){        // function for setting up a new save file at midnight
    QFile fileInput(saveFileName);
    fileInput.close();

    QThread::sleep(1);
    QString name = ui->LEfileName->text();
    saveFileName = name.left(name.lastIndexOf(QChar('/'))+1) + QDateTime::currentDateTime().toString("dd-MM-yy") + "_" + QString::number( hackConfig.rxFreq/uint64_t (1000000)) + "MHz.csv";
    // previous line renames current name of file with new date
    ui->LEfileName->setText(saveFileName);

    saveMeasInfo(samples);
}

void MainWindow::saveMeasInfo(double freq[]){        // writes information about date, central frequency and span
    QFile fileInput(saveFileName);

    if (fileInput.open(QIODevice::WriteOnly)){
        QTextStream textStream (&fileInput);

        textStream << "Saved data from: \t" << QDateTime::currentDateTime().toString("dd. MM. yyyy") <<
                      "\nCentral frequency: \t" << hackConfig.rxFreq/1000 << "\tkHz\n"<<
                      "Span:\t" << hackConfig.sampleRate/1000000 << "\tMHz\n\n";

        textStream.setRealNumberPrecision(8);
        textStream <<  "Frequency [kHz]\t";

        for (int i=0; i<1023; i++){     // missing one component as max count of csv columns is 1024!!!
            textStream << freq[i]/1000 << "\t";     // saving in kHz
        }
        textStream<< "\n";


        fileInput.flush();
    }
}

void MainWindow::on_PBsaveStop_clicked()
{
    if (saveEnabled){
        saveEnabled = false;
        ui->LEfreq->setDisabled(false);
        ui->PBsetFreq->setDisabled(false);
        ui->PBsaveStart->setDisabled(false);
        ui->PBsaveStop->setDisabled(true);
        ui->labelSaving->hide();

        QFile fileInput(saveFileName);
        fileInput.close();

        QString info = "Saving succesful\nFile: " + saveFileName + "\nNo. of records: " + QString::number(saveCounter) + "\nFile size: " + QString::number( fileInput.size()/1023) + "kB";
        QMessageBox::information(this,"Saving finished", info);\
        saveFileName = "";
    }
}

void MainWindow::on_LEfileName_textChanged(const QString &arg1)    // checking whether file from path exists
{
    QFile fileInput(arg1);    
    if (fileInput.exists())
        ui->labelFileExists->show();
    else
        ui->labelFileExists->hide();

    ui->PBsaveStart->setDisabled(false);
}

void MainWindow::on_PBzeroSpan_clicked()
{
    zeroSpan *winZeroSpan = new zeroSpan(this);
    winZeroSpan->setWindowTitle("Zero Frequency Span");
    winZeroSpan->exec();
}

void MainWindow::on_PBloadData_clicked()
{
    LoadData *dialog5 = new LoadData(this);
    dialog5->setWindowTitle("Display data from file");
    dialog5->exec();
}

void MainWindow::on_SliderColorScale_valueChanged(int value)
{
    colorMap->setDataRange(QCPRange(-100, value));
    ui->waterfall->replot();
    ui->LEupperWF->setText(QString::number(value));
}

void MainWindow::on_CBsampleRate_currentIndexChanged(const QString &arg1)
{
    if (hackConfig.hackrf_connected){
        hackConfig.sampleRate = arg1.toDouble() * freqUnits;
        hackrf_set_sample_rate(hackConfig.radioID, hackConfig.sampleRate);
    }
}

void MainWindow::on_CBwinShape_currentIndexChanged(int index)   // applying correct FFT filter
{
    // change of FFT window by choosing the new type
    hackConfig.filterShape = index;
    defineWindow(fftFiltr, hackConfig.filterShape);
}

void MainWindow::on_CBunits_currentIndexChanged(int index)  // setting frequency units to accept kHz, MHz, GHz
{
    switch (index) {
    case 0:
        freqUnits = 1;
        break;

    case 1:
        freqUnits = 1e3;
        break;

    case 2:
        freqUnits = 1e6;
        break;

    case 3:
        freqUnits = 1e9;
        break;
    default:
        break;
    }

    ui->fftGraph->xAxis->setLabel("Frequency ["+ui->CBunits->currentText() + "]");
    ui->fftGraph->replot();
    ui->waterfall->xAxis->setLabel("Frequency ["+ui->CBunits->currentText() + "]");
    ui->waterfall->replot();

    // If units are GHz, allow LEfreq to display double
    if (index == 3)
        ui->LEfreq->setText(QString::number(double(hackConfig.rxFreq)/freqUnits, 'f', 3));
    else
        ui->LEfreq->setText(QString::number(hackConfig.rxFreq/freqUnits));
}

void MainWindow::on_LEfreq_textChanged(const QString &arg1)
{
    // Treating frequency out of range values

    if ((arg1.toDouble()*freqUnits < 1000000) | (arg1.toDouble()*freqUnits > 6000000000)){
        ui->LEfreq->setStyleSheet("QLineEdit {background-color: rgb(255, 117, 117);}");     // Line edit turns red
        ui->PBsetFreq->setDisabled(true);
        ui->label_5->setText("Frequency has to be in range of 1MHz - 6GHz");
        ui->label_5->show();
    }
    else
    {
        if ( uint64_t(arg1.toDouble()*freqUnits) == hackConfig.rxFreq){  // puvodni argument 1: arg1.toUInt()
            ui->LEfreq->setStyleSheet("QLineEdit {background-color: white;}");
            ui->label_5->hide();
            }
        else{
            ui->LEfreq->setStyleSheet("QLineEdit {background-color: rgb(230, 255, 204);}");
            ui->label_5->setText("Frequency not set yet");
            ui->label_5->show();
        }

        if (hackConfig.hackrf_connected)    // check whether radio is connected before enabling the button
            ui->PBsetFreq->setDisabled(false);
    }
}
