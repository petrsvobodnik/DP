#include "zerospan.h"
#include "ui_zerospan.h"
#include "mainwindow.h"
#include <QDebug>

QString filename, dateOfMeas;
int rowCount = 0, columnCount = 0, rowTotal;
int rowOffset = 6, columnOffset = 1;




zeroSpan::zeroSpan(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::zeroSpan)
{
    ui->setupUi(this);

    ui->graphWidget->addGraph();
    ui->graphWidget->graph(0)->setPen(QPen(Qt::blue));
    ui->graphWidget->graph(0)->setName("Single frequency plot");
    ui->graphWidget->yAxis->setRange(-100, -20);

    ui->graphWidget->setInteractions(QCP::iRangeZoom | QCP::iSelectPlottables| QCP::iRangeDrag);
    ui->graphWidget->axisRect()->setRangeDrag(Qt::Horizontal);
    ui->graphWidget->axisRect()->setRangeZoom(Qt::Horizontal);

    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat("hh:mm:ss.zzz");
    ui->graphWidget->xAxis->setTicker(dateTicker);
    ui->graphWidget->yAxis->setRange(-100, -20);


    ui->SBnoOfSamples->setDisabled(true);
    ui->PBplot->setDisabled(true);
}

zeroSpan::~zeroSpan()
{
    delete ui;
}

void zeroSpan::on_PBload_clicked()
{
    filename = QFileDialog::getOpenFileName(this, "Choose file with measured data", "/home/golem/Downloads", "CSV file (*.csv)");   // QDir::homePath())
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            qDebug() << "File not open!";
            return;
    }


    QStringList frequenz;
    QString line, freq, span;
    QTextStream stream(&file);

    while (stream.readLineInto(&line)){
        frequenz = line.split('\t');

        switch (rowCount) {
        case 0:
            // ui display date
            dateOfMeas = frequenz.at(1);
            ui->labelDate->setText(ui->labelDate->text() + dateOfMeas);
            qDebug() << frequenz.at(1);
            break;
        case 1:
            // ui display central freq
            freq = frequenz.at(1);
            ui->labelFrequency->setText(ui->labelFrequency->text() + freq + " kHz");
            break;
        case 2:
            // ui display span
            span = frequenz.at(1);
            ui->labelSpan->setText(ui->labelSpan->text() + span + " MHz");
            ui->labelFreqRes->setText(ui->labelFreqRes->text().append(QString::number(span.toInt()/0.001024)+" Hz"));
            break;
        case 3:
            break;
        case 4:
            frequenz.removeFirst();
            ui->CBfreq->addItems(frequenz);
            break;
        default:
            ui->CBtime->addItem(frequenz.at(0));
            break;
        }

        rowCount++;
    }

    rowTotal = rowCount - rowOffset;
    file.close();
    // Frequency range
    QString range = QString::number(freq.toInt()-span.toInt()*500) + " - " + QString::number(freq.toInt() + span.toInt()*500) + " kHz";
    ui->labelFreqRange->setText(ui->labelFreqRange->text().append(range));

    ui->SBnoOfSamples->setMaximum(rowTotal);
    ui->SBnoOfSamples->setDisabled(false);
    ui->SBnoOfSamples->setValue(rowTotal);

    ui->labelNoOfMeas->setText(ui->labelNoOfMeas->text().append(QString::number(rowTotal)));
    QString duration = ui->CBtime->itemText(0).left(8)+ " - " + ui->CBtime->itemText(rowTotal).left(8);
    ui->labelDuration->setText(ui->labelDuration->text().append(duration));


    ui->PBplot->setDisabled(false);
    return;
}

void zeroSpan::on_PBplot_clicked()
{
    int idxFreq = ui->CBfreq->currentIndex()+1;
    int idxTime = ui->CBtime->currentIndex();
    int idxStart = idxTime + rowOffset;
    int noOfSamples = ui->SBnoOfSamples->value();

    QVector<double> dataFreq;
    QVector<double> dataTime;

    rowCount = 1;

    QFile file(filename);


    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "File not opened correctly!";
        return;
    }
    file.seek(0);

    QString line, valueFreq, valueTime;
    QStringList frequenz;

    QTextStream stream(&file);
    while (stream.readLineInto(&line)){
        rowCount++;
        if ((rowCount > idxStart) && (rowCount < idxStart + noOfSamples)){
            frequenz = line.split('\t');
            valueTime = frequenz.at(0)+" "+dateOfMeas;
//            valueTime = frequenz.at(0);
            dataTime.append((QDateTime::fromString(valueTime, "hh:mm:ss.zzz dd. MM. yyyy")).toTime_t());       // jak predat grafu datum?

            valueFreq = frequenz.at(idxFreq);
            dataFreq.append(valueFreq.toDouble());

        }
    }
    file.close();

    ui->graphWidget->graph(0)->setData(dataTime, dataFreq);
    ui->graphWidget->xAxis->setRange(dataTime.first()-10, dataTime.last()+10);

    ui->graphWidget->replot();
//    ui->graphWidget->update();
}
