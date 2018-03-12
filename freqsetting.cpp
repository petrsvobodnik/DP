#include "freqsetting.h"
#include "ui_freqsetting.h"
#include "hackrf.h"
#include "mainwindow.h"
#include <QDebug>

extern radio_config hackConfig;

freqSetting::freqSetting(QWidget *parent) :
    // Window constructor
    QDialog(parent),
    ui(new Ui::freqSetting)
{
    ui->setupUi(this);

    // allowing defined values of gains and interconnection of slider and spinbox
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


    // Deactivate buttons when Hack is not connected
    ui->PBapply->setDisabled(!hackConfig.hackrf_connected);
    ui->PBok->setDisabled(!hackConfig.hackrf_connected);


    // Load current values
    ui->LNAslider->setValue(hackConfig.LNAgain/8);
    ui->VGAslider->setValue(hackConfig.VGAgain/2);
    ui->CheckBantPow->setChecked(hackConfig.antPower);
}

freqSetting::~freqSetting()
{
    delete ui;
}

void freqSetting::setRadio(hackrf_device *id){
    sdr = id;
}


// treating gain values
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
    // setting the values and printing into console

//    uint32_t LNAgain = 8* uint32_t (ui->LNAslider->value());
    hackConfig.LNAgain = 8* uint32_t (ui->LNAslider->value());
    hackrf_set_lna_gain(sdr, hackConfig.LNAgain);
    ui->console->setText("LNA gain set "+ QString::number(hackConfig.LNAgain) + " dB");

//    uint32_t VGAgain = 2*uint32_t (ui->VGAslider->value());
    hackConfig.VGAgain = 2*uint32_t (ui->VGAslider->value());
    hackrf_set_vga_gain(sdr,hackConfig.VGAgain);
    ui->console->append("VGA gain set to: " + QString::number(hackConfig.VGAgain) + "dB");

//    const uint8_t antPow = uint8_t (ui->powerportPB->isChecked());
    hackConfig.antPower = uint8_t (ui->CheckBantPow->isChecked());
    hackrf_set_antenna_enable(sdr, hackConfig.antPower);
    if (hackConfig.antPower)
        ui->console->append("ant pow enabled");
    else
        ui->console->append("ant pow disabled");
}

void freqSetting::on_PBok_clicked()
{
    on_PBapply_clicked();
    on_PBcancel_clicked();
}
