#include "rfusetting.h"
#include "ui_rfusetting.h"
#include "mainwindow.h"
#include <QDebug>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

extern radio_config hackConfig;
QSerialPort port;

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

    ui->RBgroupPOLARISATION->setId(ui->RBpolarisationH, 1);
    ui->RBgroupPOLARISATION->setId(ui->RBpolarisationV, 2);

    connect(ui->RBgroupLEVEL, SIGNAL(buttonClicked(int)), this, SLOT(RBgroupLEVEL_clicked(int)));
    connect(ui->RBgroupLEVEL, SIGNAL(buttonClicked(int)), this, SLOT(computeGain()));
    connect(ui->RBgroupANT, SIGNAL(buttonClicked(int)), this, SLOT(RBgroupANT_clicked(int)));
    connect(ui->RBgroupANT, SIGNAL(buttonClicked(int)), this, SLOT(computeGain()));
    connect(ui->RBgroupFILT, SIGNAL(buttonClicked(int)), this, SLOT(RBgroupFILT_clicked(int)));
    connect(ui->RBgroupFILT, SIGNAL(buttonClicked(int)), this, SLOT(computeGain()));
    connect(ui->SBlevel, SIGNAL(valueChanged(int)), this, SLOT(computeGain()));

    ui->RBant1->setChecked(true);
    ui->LEantGain->setText(QString::number(0));

    ui->RBlevelATT->setChecked(true);
    ui->SBlevel->setValue(-55);
    ui->SBazimuth->setValue(0);

    ui->RBfiltBypass->setChecked(true);
    ui->LEfiltGain->setText(QString::number(0));
    ui->LEfiltGain->setReadOnly(true);
    ui->LEtotalGain->setAlignment(Qt::AlignCenter);
    ui->LEtotalGain->setReadOnly(true);
    ui->LEantGain->setReadOnly(true);
    computeGain();

    ui->label_6->hide();


    ui->GBpolarisation->setDisabled(true);
    ui->GBazimuth->setDisabled(true);

    QStringList listOfPorts;
    QList <QSerialPortInfo> portInfo = QSerialPortInfo::availablePorts();
    for (int i=0; i<portInfo.length(); i++)
        listOfPorts.append(portInfo[i].portName());
    ui->CBserialPortAR->addItems(listOfPorts);

    if (portInfo.length()==0){
        qDebug() << "Names of ports are made up";
        ui->CBserialPortAR->addItem("COM1");
        ui->CBserialPortAR->addItem("COM2");
        ui->CBserialPortAR->addItem("COM3");
    }
}

RFUsetting::~RFUsetting()
{
    delete ui;
    port.close();
}

void RFUsetting::on_PBok_clicked()
{
    hackConfig.RFUgain = ui->LEtotalGain->text().toFloat();
    qDebug() << "RFU's gain is: " << hackConfig.RFUgain;
    this->close();
}

void RFUsetting::on_PBgetGain_clicked() // SET ALL
{
//    This  button will set  RF unit -- will send commands to microcontroller

}

void RFUsetting::on_PBstopRotator_clicked()
{
    // send command S, pak zjistit stav a obnovit SBazimuth a SBelevation
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
    port.write("L");
}

void RFUsetting::on_PBrotLEFT_released()
{
    // send S
    ui->label_6->hide();
    port.write("S");
    getRotatorState();
}

void RFUsetting::on_PBrotRIGHT_pressed()
{
    // R
    ui->label_6->show();
    port.write("R");
}

void RFUsetting::on_PBrotRIGHT_released()
{
    // send S
    ui->label_6->hide();
    port.write("S");
    getRotatorState();
}

void RFUsetting::RBgroupLEVEL_clicked(int button){  // choice of RadioButtons
    int ampGain = 40;

    switch (button) {
    case 1: // AMP
        ui->SBlevel->setMaximum(ampGain);
        ui->ATTslider->setMaximum(ampGain);
        ui->SBlevel->setValue(ampGain);
        ui->SBlevel->setReadOnly(1);
        ui->ATTslider->hide();
        break;

    case 2: // ATT
        ui->SBlevel->setMaximum(0);
        ui->ATTslider->setMaximum(0);
        ui->SBlevel->setReadOnly(0);
        ui->SBlevel->setValue(-55);
        ui->ATTslider->show();
        break;

    case 3: // Bypass
        ui->SBlevel->setReadOnly(1);
        ui->ATTslider->hide();
        ui->SBlevel->setValue(0);
        break;

    default:
        break;

    }
}

void RFUsetting::RBgroupANT_clicked(int button){
    // tady bude potreba znat parametry anteny a volat fci, ktera vypocita zisk
    ui->LEantGain->setText(QString::number(-10*button));

}

void RFUsetting::RBgroupFILT_clicked(int button){
    // to same jak u anteny
    ui->LEfiltGain->setText(QString::number(-10*button));

}

void RFUsetting::computeGain(){
    int gain = ui->LEfiltGain->text().toInt() + ui->LEfiltGain->text().toInt() + ui->SBlevel->value();
    ui->LEtotalGain->setText(QString::number(gain));
}

void RFUsetting::on_PBconnectAR_clicked()
{
    ui->GBazimuth->setDisabled(false);
    QMessageBox::information(this, "Connection established", "Connected to "+ui->CBserialPortAR->currentText());
    // rovnou vycist uhel natoceni rotatoru a polarizaci

    port.setPortName(ui->CBserialPortAR->currentText());
    port.open(QIODevice::ReadWrite);
    port.setBaudRate(QSerialPort::Baud9600);
    port.setDataBits(QSerialPort::Data8);
    port.setParity(QSerialPort::NoParity);
    port.setStopBits(QSerialPort::OneStop);
    port.setFlowControl(QSerialPort::NoFlowControl);
    getRotatorState();

}

void RFUsetting::on_PBsetpolarisation_clicked()
{

}

void RFUsetting::getRotatorState(){
    int readAngle;
    port.write("C");
    // osetrit cteni z portu a nastaveni hodnoty do GUI
//    ui->SBazimuth->setValue(readAngle/1024*360);
}
