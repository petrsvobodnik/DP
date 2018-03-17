#include "rfuwindow.h"
#include "ui_rfuwindow.h"

RFUwindow::RFUwindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RFUwindow)
{
    ui->setupUi(this);
}

RFUwindow::~RFUwindow()
{
    delete ui;
}
