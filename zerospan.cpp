#include "zerospan.h"
#include "ui_zerospan.h"
#include "mainwindow.h"
#include <QDebug>

QString filename, dateOfMeas;
int rowCount = 0, columnCount = 0, rowTotal;
int rowOffset = 6, columnOffset = 1;


zeroSpan::zeroSpan(QWidget *parent) :
    QDialog(parent),
   ui(new Ui::zeroSpan) // constructor
{
    ui->setupUi(this);

    ui->graphWidget->addGraph();
    ui->graphWidget->graph(0)->setPen(QPen(Qt::blue));
    ui->graphWidget->graph(0)->setName("Single frequency plot");
    ui->graphWidget->yAxis->setRange(-100, -20);
    ui->graphWidget->yAxis->setLabel("Power [dBm]");
    ui->graphWidget->xAxis->setLabel("Time [hh:mm:ss]");

    ui->graphWidget->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);
    ui->graphWidget->axisRect()->setRangeDrag(Qt::Horizontal);
    ui->graphWidget->axisRect()->setRangeZoom(Qt::Horizontal);

    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat("hh:mm:ss");
    ui->graphWidget->xAxis->setTicker(dateTicker);
    ui->graphWidget->yAxis->setRange(-100, -20);
        ui->SBlowerRange->setValue(ui->graphWidget->yAxis->range().lower);
    ui->SBupperRange->setValue(ui->graphWidget->yAxis->range().upper);
    ui->LEfreq->setReadOnly(true);

    ui->SBnoOfSamples->setDisabled(true);
    ui->PBplot->setDisabled(true);
    ui->SliderSelectFreq->setDisabled(true);

    this->setFixedSize(800, 615);
}

zeroSpan::~zeroSpan()
{
    delete ui;
}

void zeroSpan::on_PBload_clicked()
{
    QTime timeStart, timeStop, timeLength;
    filename = QFileDialog::getOpenFileName(this, "Choose file with measured data", "/home/golem/Downloads", "CSV file (*.csv)");   // QDir::homePath())
    ui->LEfilePath->setText(filename);
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            qDebug() << "File not open!";
            return;
    }

    file.seek(0);

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
    // Frequency range ui label
    QString range = QString::number(freq.toInt()-span.toInt()*500) + " - " + QString::number(freq.toInt() + span.toInt()*500) + " kHz";
    ui->labelFreqRange->setText(ui->labelFreqRange->text().append(range));

    ui->SBnoOfSamples->setMaximum(rowTotal);
    ui->SBnoOfSamples->setDisabled(false);
    ui->SBnoOfSamples->setValue(rowTotal);

    ui->labelNoOfMeas->setText(ui->labelNoOfMeas->text().append(QString::number(rowTotal)));

    QString time1 = ui->CBtime->itemText(0);
    QString time2 = ui->CBtime->itemText(rowTotal);

    timeStart= QTime::fromString(time1, "hh:mm:ss.zzz");
    timeStop = QTime::fromString(time2, "hh:mm:ss.zzz");
    timeLength.setHMS((timeStop.hour()-timeStart.hour()), (timeStop.minute()-timeStart.minute()), (timeStop.second()-timeStart.second()));

    QString duration = timeStart.toString("hh:mm:ss")+ " - " + timeStop.toString("hh:mm:ss");
    ui->labelDuration->setText(ui->labelDuration->text().append(duration));

//    qDebug() << "Start: " << timeStart.toString("hh:mm:ss.zzz") << "\nEnd: " << timeStop.toString("hh:mm:ss.zzz") <<
//                "\nLength: " << timeLength.toString("hh:mm:ss");

    ui->labelMeasLength->setText(ui->labelMeasLength->text()+timeLength.toString("hh:mm:ss"));


    ui->PBplot->setDisabled(false);
    ui->CBfreq->setCurrentIndex(1);
    ui->SliderSelectFreq->setDisabled(false);
    ui->SliderSelectFreq->setMaximum(ui->CBfreq->count());
    ui->LEfreq->setReadOnly(true);
    on_PBplot_clicked();
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
        QMessageBox::warning(this, "Error", "File not open!");
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

void zeroSpan::on_SBupperRange_valueChanged(int arg1)
{
    ui->graphWidget->yAxis->setRangeUpper(arg1);
    ui->graphWidget->replot();
}

void zeroSpan::on_SBlowerRange_valueChanged(int arg1)
{
    ui->graphWidget->yAxis->setRangeLower(arg1);
    ui->graphWidget->replot();
}

void zeroSpan::on_CBtime_currentIndexChanged(int index)
{
    ui->SBnoOfSamples->setMaximum(rowTotal-index);
    ui->SBnoOfSamples->setValue(rowTotal-index);
}

void zeroSpan::on_CBfreq_currentIndexChanged(int index)
{
    static int x;
    if (x>0){
        on_PBplot_clicked();
        ui->SliderSelectFreq->setValue(index);
        ui->LEfreq->setText(ui->CBfreq->currentText());
    }
    x++;
}

void zeroSpan::on_SliderSelectFreq_valueChanged(int value)
{
    static int x;
    if (x>0){
        ui->CBfreq->setCurrentIndex(value);
        ui->LEfreq->setText(ui->CBfreq->currentText());
        on_PBplot_clicked();
    }
    x++;
}

