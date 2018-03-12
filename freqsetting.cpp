#include "freqsetting.h"
#include "ui_freqsetting.h"
#include "hackrf.h"
#include "mainwindow.h"
#include <QDebug>



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

    uint32_t LNAgain = uint32_t (ui->LNAslider->value());
    hackrf_set_lna_gain(sdr, 8*LNAgain);
    ui->console->setText("LNA gain set "+ QString::number(8*ui->LNAslider->value()) + " dB");

    uint32_t VGAgain = uint32_t (ui->VGAslider->value());
    hackrf_set_vga_gain(sdr,2*VGAgain);
    ui->console->append("VGA gain set to: " + QString::number(2*ui->VGAslider->value()) + "dB");

    const uint8_t antPow = uint8_t (ui->powerportPB->isChecked());
    hackrf_set_antenna_enable(sdr, antPow);
    if (antPow)
        ui->console->append("ant pow enabled");
    else
        ui->console->append("ant pow disabled");
}

void freqSetting::on_PBok_clicked()
{
    on_PBapply_clicked();
    on_PBcancel_clicked();
}
