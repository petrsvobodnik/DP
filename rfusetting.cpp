#include "rfusetting.h"
#include "ui_rfusetting.h"
#include "mainwindow.h"

extern radio_config hackConfig;

RFUsetting::RFUsetting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RFUsetting)
{
    ui->setupUi(this);
    ui->LEfreq->setText(QString::number(hackConfig.rxFreq/1000));
    ui->LEfreq->setAlignment(Qt::AlignRight);
    ui->label->setText("Frequency [kHz]");
}

RFUsetting::~RFUsetting()
{
    delete ui;
}

void RFUsetting::on_PBok_clicked()
{
    on_PBapply_clicked();
    this->close();
}

void RFUsetting::on_PBapply_clicked()
{

}

void RFUsetting::on_PBgetGain_clicked()
{

}
