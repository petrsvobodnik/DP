#include "freqsetting.h"
#include "ui_freqsetting.h"
#include "hackrf.h"
#include "mainwindow.h"
#include <QDebug>

freqSetting::freqSetting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::freqSetting)
{
    ui->setupUi(this);


    ui->VGAslider->setRange(0, 31);
    ui->VGAslider->setSingleStep(2);
    ui->VGAslider->setTickInterval(10);
    ui->VGAslider->setTickPosition(QSlider::TicksBelow);

    ui->VGAspinBox->setRange(0, 62);
    ui->VGAspinBox->setSingleStep(2);

    connect(ui->VGAslider, SIGNAL(valueChanged(int)), this, SLOT(slidSpinVGA(int)));
    connect(ui->VGAspinBox, SIGNAL(valueChanged(int)), this, SLOT(spinSlidVGA(int)));


    ui->LNAslider->setRange(0, 5);
    ui->LNAslider->setSingleStep(8);
    ui->LNAslider->setTickInterval(1);
    ui->LNAslider->setTickPosition(QSlider::TicksBelow);

    ui->LNAspinBox->setRange(0, 40);    // IF gain
    ui->LNAspinBox->setSingleStep(8);

    connect(ui->LNAslider, SIGNAL(valueChanged(int)), this, SLOT(slidSpinLNA(int)));
    connect(ui->LNAspinBox, SIGNAL(valueChanged(int)), this, SLOT(spinSlidLNA(int)));

}

freqSetting::~freqSetting()
{
    delete ui;
}

void freqSetting::setRadio(hackrf_device *id){
    sdr = id;
}


// Osetreni hodnot zisku
void freqSetting::slidSpinVGA(int gain){
    ui->VGAspinBox->setValue(2*gain);
}

void freqSetting::spinSlidVGA(int gain){
    ui->VGAslider->setValue(gain/2);
}

void freqSetting::slidSpinLNA(int gain){
    ui->LNAspinBox->setValue(8*gain);
}

void freqSetting::spinSlidLNA(int gain){
    ui->LNAslider->setValue(gain/8);
}



void freqSetting::on_PBcancel_clicked()
{
     window()->close();
}

void freqSetting::on_PBapply_clicked()
{
    uint32_t LNAgain = uint32_t (ui->LNAslider->value());
    hackrf_set_lna_gain(sdr, 8*LNAgain);
    qDebug() << "LNA gain set to " << 8*LNAgain << " dB" ;
    ui->konzole->setText("LNA gain set "+ QString {ui->LNAslider->value()} + " dB");

    uint32_t VGAgain = uint32_t (ui->VGAslider->value());
    hackrf_set_vga_gain(sdr,2*VGAgain);
    ui->konzole->append("VGA gain set");

    const uint8_t antPow = uint8_t (ui->powerportPB->isChecked());
    hackrf_set_antenna_enable(sdr, antPow);
    if (antPow)
        ui->konzole->append("ant pow enabled");
    else
        ui->konzole->append("ant pow disabled");
}
