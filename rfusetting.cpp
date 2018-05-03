#include "rfusetting.h"
#include "ui_rfusetting.h"
#include "mainwindow.h"
#include <QDebug>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

extern radio_config hackConfig;
QSerialPort portAR, portAU;
QTimer readPositionTimer;
QVector<double> filtFreq, filtVal, antFreq, antVal, ampFreq, ampVal;

RFUsetting::RFUsetting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RFUsetting)
{
    ui->setupUi(this);
    ui->LEfreq->setText(QString::number(hackConfig.rxFreq/1000));
    ui->LEfreq->setAlignment(Qt::AlignRight);
    ui->LEfreq->setReadOnly(true);
    ui->label->setText("Frequency [kHz]");

    ui->SBazimuth->setValue(0);

    ui->RBgroupLEVEL->setId(ui->RBlevelAMP, 1);
    ui->RBgroupLEVEL->setId(ui->RBlevelATT, 2);
    ui->RBgroupLEVEL->setId(ui->RBlevelBypass, 3);
    ui->RBgroupLEVEL->setId(ui->RBlevelfree, 4);

    ui->RBgroupPOLARISATION->setId(ui->RBpolarisationH, 1);
    ui->RBgroupPOLARISATION->setId(ui->RBpolarisationV, 2);

    ui->RBgroupFILT->setId(ui->RBfiltBypass, 1);
    ui->RBgroupFILT->setId(ui->RBfiltLP, 2);
    ui->RBgroupFILT->setId(ui->RBfiltHP, 3);
    ui->RBgroupFILT->setId(ui->RBfiltFM, 4);
    ui->RBgroupFILT->setId(ui->RBfiltGSM, 5);
    ui->RBgroupFILT->setId(ui->RBfiltfree, 6);

    ui->RBgroupANT->setId(ui->RBant1, 1);
    ui->RBgroupANT->setId(ui->RBant2, 2);
    ui->RBgroupANT->setId(ui->RBant3, 3);
    ui->RBgroupANT->setId(ui->RBantfree, 4);


    connect(ui->RBgroupLEVEL, SIGNAL(buttonClicked(int)), this, SLOT(RBgroupLEVEL_clicked(int)));
    connect(ui->RBgroupLEVEL, SIGNAL(buttonClicked(int)), this, SLOT(computeGain()));
    connect(ui->RBgroupANT, SIGNAL(buttonClicked(int)), this, SLOT(RBgroupANT_clicked(int)));
    connect(ui->RBgroupANT, SIGNAL(buttonClicked(int)), this, SLOT(computeGain()));
    connect(ui->RBgroupFILT, SIGNAL(buttonClicked(int)), this, SLOT(RBgroupFILT_clicked(int)));
    connect(ui->RBgroupFILT, SIGNAL(buttonClicked(int)), this, SLOT(computeGain()));
    connect(ui->SBlevel, SIGNAL(valueChanged(double)), this, SLOT(computeGain()));
    connect(ui->LEantGain, SIGNAL(textChanged(QString)), this, SLOT(computeGain()));
    connect(ui->LEfiltGain, SIGNAL(textChanged(QString)), this, SLOT(computeGain()));


//    ui->RBant1->setChecked(true);
//    ui->LEantGain->setText(QString::number(0));

//    ui->RBlevelATT->setChecked(true);
//    ui->SBlevel->setValue(-55);
    ui->SBazimuth->setValue(0);

//    ui->RBfiltBypass->setChecked(true);
//    ui->LEfiltGain->setText(QString::number(0));
    ui->LEfiltGain->setReadOnly(true);
    ui->LEtotalGain->setAlignment(Qt::AlignCenter);
    ui->LEtotalGain->setReadOnly(true);
    ui->LEantGain->setReadOnly(true);
    computeGain();

    ui->label_6->hide();

    ui->SBswitchLoss->setValue(0.2);
    ui->SBswitchLoss->setSingleStep(0.1);
    connect(ui->SBswitchLoss, SIGNAL(valueChanged(double)), this, SLOT(computeGain()));
    connect(ui->SBswitchCount, SIGNAL(valueChanged(int)), this, SLOT(computeGain()));


    ui->GBpolarisation->setDisabled(true);
    ui->GBazimuth->setDisabled(true);

    QStringList listOfPorts;
    QList <QSerialPortInfo> portInfo = QSerialPortInfo::availablePorts();
    for (int i=0; i<portInfo.length(); i++)
        listOfPorts.append(portInfo[i].portName());
    ui->CBserialPortAR->addItems(listOfPorts);
    ui->CBserialPortAU->addItems(listOfPorts);

//    if (portInfo.length()==0){
//        qDebug() << "Names of ports are made up";
//        ui->CBserialPortAR->addItem("COM1");
//        ui->CBserialPortAR->addItem("COM2");
//        ui->CBserialPortAR->addItem("COM3");
//    }

    connect(&readPositionTimer, SIGNAL(timeout()), this, SLOT(getRotatorState()));
}

RFUsetting::~RFUsetting()
{
    delete ui;
    portAR.close();
    portAU.close();
}

void RFUsetting::on_PBok_clicked()
{
    if (hackConfig.RFUgain != ui->LEtotalGain->text().toFloat()){
        QMessageBox::information(this, "Parameters set", "Last change of antenna unit was set");
        on_PBgetGain_clicked();     // calling the function for changing of unit's settings
    }


    qDebug() << "RFU's gain is: " << hackConfig.RFUgain;
    this->close();
}

void RFUsetting::on_PBgetGain_clicked() // SET ALL
{
    if ((ui->RBgroupFILT->checkedId()== -1) | (ui->RBgroupANT->checkedId()== -1) | (ui->RBgroupLEVEL->checkedId()== -1 )){
        QMessageBox::warning(this, "Not complete", "You need to choose all the options!");
        return;
    }
        hackConfig.RFUgain = ui->LEtotalGain->text().toFloat();
//    This  button will set  RF unit -- will send commands to microcontroller

}

void RFUsetting::on_PBstopRotator_clicked()
{
    // send command S, pak zjistit stav a obnovit SBazimuth
    getRotatorState();
}


void RFUsetting::on_PBsetRotator_clicked()
{
    // send W xxx yyy for setting the angles (xxx = Azim. Angle; yyy = Elev. Angle). Example: W350 163
    int x = 360; // range of azimuth rotation
    int xx = ui->SBazimuth->value();    // help variable
    int xxx;


    if (xx>=0)
        xxx = xx * 1024/x;
    else
        xxx = (xx+360) * 1024/x;

//    port.write("M"+QString::number(xxx));

    qDebug() << "Azimuth: " << xxx ;
}


void RFUsetting::on_PBrotLEFT_pressed()
{
    // L timerem cyklicky obnovovat hodnotu azimutu
    ui->label_6->show();
    portAR.write("L");
    readPositionTimer.start(200);
}

void RFUsetting::on_PBrotLEFT_released()
{
    // send S
    ui->label_6->hide();
    portAR.write("S");
    readPositionTimer.stop();
}

void RFUsetting::on_PBrotRIGHT_pressed()
{
    // R
    ui->label_6->show();
    portAR.write("R");
    readPositionTimer.start(200);
}

void RFUsetting::on_PBrotRIGHT_released()
{
    // send S
    ui->label_6->hide();
    portAR.write("S");
    readPositionTimer.stop();
}

void RFUsetting::RBgroupLEVEL_clicked(int button){  // choice of RadioButtons

    switch (button) {
    case 1: // AMP        
        ui->SBlevel->setReadOnly(true);
        ui->ATTslider->hide();

        readParameters(":/filters/data/Zes_0-6GHz.txt", &ampFreq, &ampVal);
        ui->SBlevel->setValue(interpolateValue(&ampFreq, &ampVal));

        break;

    case 2: // ATT
        ui->ATTslider->setMaximum(0);
        ui->SBlevel->setRange(-55,0);
        ui->SBlevel->setReadOnly(false);
        ui->SBlevel->setValue(-55);
        ui->ATTslider->show();
        break;

    case 3: // Bypass
        ui->SBlevel->setReadOnly(true);
        ui->ATTslider->hide();
        ui->SBlevel->setValue(0);
        break;

    case 4:
        ui->SBlevel->setReadOnly(false);
        ui->SBlevel->setValue(0);
        ui->SBlevel->setRange(-50, 50);
        ui->ATTslider->hide();

    default:
        break;

    }
}

void RFUsetting::RBgroupANT_clicked(int button){
    // tady bude potreba znat parametry anteny a volat fci, ktera vypocita zisk
    ui->LEantGain->setReadOnly(true);
    QString nameOfFile;

    switch (button) {
    case 1: // HK309
        nameOfFile = ":/filters/data/anteny/HK309_0.2-1.3.txt";
        break;

    case 2: // HL223
        nameOfFile = ":/filters/data/anteny/HL223_0.2-1.3.txt";
        break;

    case 3:
        nameOfFile = "";
        break;

    case 4:
        nameOfFile = QFileDialog::getOpenFileName(this, "Select file...", QDir::homePath());
        break;

    default:
        break;
    }

    if (nameOfFile != ""){
        readParameters(nameOfFile, &antFreq, &antVal);
        double gain = interpolateValue(&antFreq, &antVal);
        ui->LEantGain->setText(QString::number(gain, 'f', 1));
    }
    else
        ui->LEantGain->setText(QString::number(0));

    computeGain();
}

void RFUsetting::RBgroupFILT_clicked(int button){
    // to same jak u anteny
    ui->LEfiltGain->setReadOnly(true);
    QString nameOfFile;

    switch (button) {
    // Assigning filepath to the file with parameters
    case 1: // bypass
        ui->LEfiltGain->setText(QString::number(0));
        nameOfFile = "";
        break;

    case 2: // LP
        nameOfFile = ":/filters/data/filtry/LP_1GHz_0-6GHz.txt";
        break;

    case 3: // HP
        nameOfFile = ":/filters/data/filtry/HP_1,1GHz_0-6GHz.txt";
        break;

    case 4: // BR FM
        nameOfFile = ":/filters/data/filtry/BS_FM_0-6GHz.txt";
        break;

    case 5: // BR LTE
        nameOfFile = "";
        break;

    case 6: // free port
        nameOfFile = QFileDialog::getOpenFileName(this, "Select file...", QDir::homePath());
        break;

    default:
        break;
    }

    if (nameOfFile != ""){
        readParameters(nameOfFile, &filtFreq, &filtVal);
        double gain = interpolateValue(&filtFreq, &filtVal);
        ui->LEfiltGain->setText(QString::number(gain, 'f', 1));
    }
    else
        ui->LEfiltGain->setText(QString::number(0));

    computeGain();
}

void RFUsetting::computeGain(){
    // summing all the gains and losses
    double gain = ui->LEfiltGain->text().toDouble() + ui->LEantGain->text().toDouble()
            + ui->SBlevel->value() - ui->SBswitchCount->value()*ui->SBswitchLoss->value();
    ui->LEtotalGain->setText(QString::number(gain, 'f', 1));
}

void RFUsetting::on_PBconnectAR_clicked()
{
    ui->GBazimuth->setDisabled(false);
//    QMessageBox::information(this, "Connection established", "Connected to "+ui->CBserialPortAR->currentText());
    // rovnou vycist uhel natoceni rotatoru a polarizaci

    portAR.open(QIODevice::ReadWrite);
    portAR.setPortName(ui->CBserialPortAR->currentText());
    portAR.setBaudRate(QSerialPort::Baud9600);
    portAR.setDataBits(QSerialPort::Data8);
    portAR.setParity(QSerialPort::NoParity);
    portAR.setStopBits(QSerialPort::OneStop);
    portAR.setFlowControl(QSerialPort::NoFlowControl);

    portAR.write("hello");
    //    getRotatorState();

}

void RFUsetting::on_PBconnectAU_clicked()
{
    QMessageBox::information(this, "Connection established", "Connected to "+ui->CBserialPortAU->currentText());
    // rovnou vycist uhel natoceni rotatoru a polarizaci

    portAU.setPortName(ui->CBserialPortAU->currentText());
    portAU.open(QIODevice::ReadWrite);
    portAU.setBaudRate(QSerialPort::Baud9600);
    portAU.setDataBits(QSerialPort::Data8);
    portAU.setParity(QSerialPort::NoParity);
    portAU.setStopBits(QSerialPort::OneStop);
    portAU.setFlowControl(QSerialPort::NoFlowControl);

}


void RFUsetting::getRotatorState(){
//    int readAngle;
    portAR.write("C");
    // osetrit cteni z portu a nastaveni hodnoty do GUI
//    ui->SBazimuth->setValue(readAngle/1024*360);
}

void RFUsetting::readParameters(QString name, QVector<double> *freq, QVector<double> *val){
    if (!freq->isEmpty()){
        freq->clear();
        val->clear();
    }

    QFile file(name);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){     // handling open file failure
            qDebug() << "File not open!";
            QMessageBox::warning(this, "Error", "File " + name + " not open!");
            return;
    }

    file.seek(0);       // return to the file beginning

    QStringList separatedLine;
    QString singleLine;
    QTextStream stream(&file);

    while (stream.readLineInto(&singleLine)){ // reading out the header of the measurement
        separatedLine = singleLine.split(';');
        freq->append(separatedLine.at(0).toDouble());
        QString value =separatedLine.at(1);
        val->append(value.left(6).toDouble());
    }

    file.close();
}


double RFUsetting::interpolateValue(QVector<double> *freq, QVector<double> *val){
    double x = ui->LEfreq->text().toDouble()*1000;
    double x0, x1, y, y0, y1;
    int index=0;

    // searching for the index of frequency
    while(freq->at(index)< x){
        index++;
    }

    if (index==0){
        y = val->at(index);
    }
    else{
        x0 = freq->at(index-1);
        x1 = freq->at(index);
        y0 = val->at(index-1);
        y1 = val->at(index);
        y = y0 + (x-x0)*(y1-y0)/(x1-x0);
    }
    return y;
}

void RFUsetting::on_SBlevel_valueChanged(double arg1)
{
    ui->ATTslider->setValue(int (arg1));
}

void RFUsetting::on_ATTslider_valueChanged(int value)
{
    ui->SBlevel->setValue(double(value));
}

