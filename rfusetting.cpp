#include "rfusetting.h"
#include "ui_rfusetting.h"
#include "mainwindow.h"
#include <QDebug>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

extern radio_config hackConfig;
QSerialPort portAR, portAU;
QTimer readPositionTimer;

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
    connect(ui->SBlevel, SIGNAL(valueChanged(int)), this, SLOT(computeGain()));
    connect(ui->LEantGain, SIGNAL(textChanged(QString)), this, SLOT(computeGain()));
    connect(ui->LEfiltGain, SIGNAL(textChanged(QString)), this, SLOT(computeGain()));


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

    ui->SBswitchLoss->setValue(0.2);
    ui->SBswitchLoss->setSingleStep(0.1);
    connect(ui->SBswitchLoss, SIGNAL(valueChanged(double)), this, SLOT(computeGain()));


    ui->GBpolarisation->setDisabled(true);
    ui->GBazimuth->setDisabled(true);

    QStringList listOfPorts;
    QList <QSerialPortInfo> portInfo = QSerialPortInfo::availablePorts();
    for (int i=0; i<portInfo.length(); i++)
        listOfPorts.append(portInfo[i].portName());
    ui->CBserialPortAR->addItems(listOfPorts);
    ui->CBserialPortAU->addItems(listOfPorts);

    if (portInfo.length()==0){
        qDebug() << "Names of ports are made up";
        ui->CBserialPortAR->addItem("COM1");
        ui->CBserialPortAR->addItem("COM2");
        ui->CBserialPortAR->addItem("COM3");
    }

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
    int ampGain = 40;
    QString filename = ":/filters/data/Zes_0-6GHz.txt";

    switch (button) {
    case 1: // AMP
        ui->SBlevel->setMaximum(ampGain);
        ui->ATTslider->setMaximum(ampGain);
        ui->SBlevel->setValue(ampGain);
        ui->SBlevel->setReadOnly(true);
        ui->ATTslider->hide();
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
    ui->LEantGain->setText(QString::number(-10*button));
    ui->LEantGain->setReadOnly(true);
    QString filename;

    switch (button) {
    case 1:
        filename = "dd";
        break;
    case 2:
        filename = "dd";
        break;
    case 3:
        filename = "dd";
        break;
    case 4:
        ui->LEantGain->setText(QString::number(0));
        ui->LEantGain->setReadOnly(false);
        break;
    default:
        break;
    }
}

void RFUsetting::RBgroupFILT_clicked(int button){
    // to same jak u anteny
    ui->LEfiltGain->setText(QString::number(-10*button));
    ui->LEfiltGain->setReadOnly(true);
    QString filename;

    switch (button) {
    case 1: // bypass
        ui->LEfiltGain->setText(QString::number(0));
        filename = "";
        break;

    case 2: // LP
        filename = ":/filters/data/filtry/LP_1GHz_0-6GHz.txt";
        break;

    case 3: // HP
        filename = ":/filters/data/filtry/HP_1,1GHz_0-6GHz.txt";
        break;

    case 4: // BR FM
        filename = ":/filters/data/filtry/BS_FM_0-6GHz.txt";
        break;

    case 5: // BR LTE
        filename = "dd";
        break;

    case 6: // free port
        ui->LEfiltGain->setText(QString::number(0));
        ui->LEfiltGain->setReadOnly(false);
        filename = "";
        break;

    default:
        break;
    }
}

void RFUsetting::computeGain(){
    // summing all the gains and losses
    double gain = ui->LEfiltGain->text().toInt() + ui->LEantGain->text().toInt()
            + ui->SBlevel->value() - 5*ui->SBswitchLoss->value();
    ui->LEtotalGain->setText(QString::number(gain, 'f', 1));
}

void RFUsetting::on_PBconnectAR_clicked()
{
    ui->GBazimuth->setDisabled(false);
    QMessageBox::information(this, "Connection established", "Connected to "+ui->CBserialPortAR->currentText());
    // rovnou vycist uhel natoceni rotatoru a polarizaci

    portAR.setPortName(ui->CBserialPortAR->currentText());
    portAR.open(QIODevice::ReadWrite);
    portAR.setBaudRate(QSerialPort::Baud9600);
    portAR.setDataBits(QSerialPort::Data8);
    portAR.setParity(QSerialPort::NoParity);
    portAR.setStopBits(QSerialPort::OneStop);
    portAR.setFlowControl(QSerialPort::NoFlowControl);
    getRotatorState();

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
