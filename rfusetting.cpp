#include "rfusetting.h"
#include "ui_rfusetting.h"
#include "mainwindow.h"

extern radio_config hackConfig;

RFUsetting::RFUsetting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RFUsetting)
{
    ui->setupUi(this);
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
