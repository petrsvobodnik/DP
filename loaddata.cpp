#include "loaddata.h"
#include "ui_loaddata.h"
#include "mainwindow.h"

QString filename1, dateOfMeas1;
int rowCount1 = 0, columnCount1 = 0, rowTotal1;
int rowOffset1 = 6, columnOffset1 = 1;
int noOfRow =100, noOfCol = 1024;
int sampleSelect;

QStringList freqList, timeList;

QVector<double> dataFreq1;      //
QVector<double> dataTime1;

QCPColorMap *colorMap1 = NULL, *usage1 = NULL;
QCPItemLine *tresholdLine1 = NULL;
QCPItemLine *timeLine = NULL;


LoadData::LoadData(QWidget *parent) :       // GUI constructor
    QDialog(parent),
    ui(new Ui::LoadData)
{
    ui->setupUi(this);

    ui->LEfilename->setReadOnly(true);


    ui->graphWidget->addGraph();
    ui->graphWidget->graph(0)->setName("FFT diagram");
    ui->SBlowerSpectrum->setValue(-100);
    ui->SBupperSpectrum->setValue(-20);
    ui->graphWidget->yAxis->setRange(ui->SBlowerSpectrum->value(), ui->SBupperSpectrum->value());
    ui->graphWidget->yAxis->setLabel("Power [dBm]");
    ui->graphWidget->xAxis->setLabel("Frequency [kHz]");
    ui->graphWidget->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);
    ui->graphWidget->axisRect()->setRangeDrag(Qt::Vertical);
    ui->graphWidget->axisRect()->setRangeZoom(Qt::Vertical);



    ui->waterfallWidget->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);
    ui->waterfallWidget->axisRect()->setRangeDrag(Qt::Vertical);
    ui->waterfallWidget->axisRect()->setRangeZoom(Qt::Vertical);
    colorMap1 = new QCPColorMap(ui->waterfallWidget->xAxis, ui->waterfallWidget->yAxis);
    colorMap1->data()->setSize(noOfCol, noOfRow);
    ui->waterfallWidget->yAxis->setRangeReversed(true);

    QCPColorScale *colorScale = new QCPColorScale(ui->waterfallWidget);
    ui->waterfallWidget->plotLayout()->addElement(0, 1, colorScale);
    colorScale->setType(QCPAxis::atRight);
    colorScale->setGradient(QCPColorGradient::gpSpectrum);
    colorScale->setDataRange(QCPRange(-100, 0));
    colorMap1->setColorScale(colorScale);
    ui->waterfallWidget->xAxis->setLabel("Frequency [kHz]");


    // time is used as ticks on Y axis
    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat("hh:mm:ss");
    ui->waterfallWidget->yAxis->setTicker(dateTicker);

    // Align y axis and colorscale with both top and bottom
    QCPMarginGroup *marginGroup = new QCPMarginGroup(ui->waterfallWidget);
    ui->waterfallWidget->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    timeLine = new QCPItemLine(ui->waterfallWidget);
    timeLine->setPen(QPen(Qt::red));
    timeLine->setVisible(false);


    // Band usage initialization
    usage1 = new QCPColorMap(ui->bandUsage->xAxis, ui->bandUsage->yAxis);
    usage1->data()->setKeySize(noOfCol);
    usage1->data()->setKeyRange(QCPRange(1,noOfCol));
    usage1->data()->setValueSize(2);
    usage1->data()->setValueRange(QCPRange(0 ,3));
    usage1->setGradient(QCPColorGradient::gpHot);
    usage1->setInterpolate(false);
    usage1->setTightBoundary(true);
    usage1->setDataRange(QCPRange(0,1));
    ui->bandUsage->xAxis->setVisible(false);
    ui->bandUsage->yAxis->setVisible(false);
    ui->bandUsage->yAxis->setRange(1.5, 2);
    ui->bandUsage->xAxis->setRange(0, noOfCol);
    ui->bandUsage->replot();


    tresholdLine1 = new QCPItemLine(ui->graphWidget);
    tresholdLine1->setVisible(false);
    tresholdLine1->setPen(QPen(Qt::red));


    ui->SBtreshold->setValue(-30);
    ui->SliderTreshold->setValue(ui->SBtreshold->value());
    ui->SBtreshold->setDisabled(true);
    ui->SliderTreshold->setDisabled(true);
    ui->SliderTime->setDisabled(true);


    connect(ui->SBtreshold, SIGNAL(valueChanged(int)), ui->SliderTreshold, SLOT(setValue(int)));
    connect(ui->SliderTreshold, SIGNAL(valueChanged(int)), ui->SBtreshold, SLOT(setValue(int)));
    connect(ui->graphWidget->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(spectrumRangeChanged(QCPRange)));
    connect(colorMap1, SIGNAL(dataRangeChanged(QCPRange)), this, SLOT(WFupdateSBRange(QCPRange)));

}

LoadData::~LoadData()
{
    delete ui;
}

void LoadData::on_PBopenFile_clicked()
{
    // After file is opened, some basic information about measurement are read out and displayed in the left side of the window
    // Also, list of times and frequencies is extracted from the file



    filename1 = QFileDialog::getOpenFileName(this, "Select file with data", "/home/golem/Downloads", "CSV file (*.csv)");   //QDir::homePath()
    ui->LEfilename->setText(filename1);

    QFile file(filename1);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){     // handling open file failure
            qDebug() << "File not open!";
            QMessageBox::warning(this, "Error", "File not open!");
            return;
    }

    file.seek(0);       // return to the file beginning

    QStringList frequenz;
    QString line, freq, span, temp;
    QTextStream stream(&file);

    while (stream.readLineInto(&line)){ // reading out the header of the measurement
        frequenz = line.split('\t');

        switch (rowCount1) {
        case 0:
            // ui display date of measurement
            temp = "Date:\n";
            dateOfMeas1 = frequenz.at(1);
            ui->labelDate->setText(temp + dateOfMeas1);
            break;

        case 1:
            // ui display central freq
            temp = "Central Frequency:\n";
            freq = frequenz.at(1);
            ui->labelFrequency->setText(temp + freq + " kHz");
            break;

        case 2:
            // ui display span and FFT resolution
            temp = "Span:\n";
            span = frequenz.at(1);
            ui->labelSpan->setText(temp + span + " MHz");

            temp = "Frequency resolution:\n";
            ui->labelFreqRes->setText(temp + QString::number(span.toInt()/0.001024)+" Hz");
            break;

        case 3:     // Empty line
            break;

        case 4:     // loads all frequencies of the measurement to freqList variable at once
            frequenz.removeFirst();
            freqList.append(frequenz);
            break;

//        case 5:
//            break;

        default:        // saves times of measurement in 'rowCount1' iterations
            timeList.append(frequenz.at(0));
            dataTime1.append((QDateTime::fromString(frequenz.at(0)+" "+dateOfMeas1, "hh:mm:ss.zzz dd. MM. yyyy")).toTime_t());
            break;
        }

        rowCount1++;
    }

  // Max count of samples in WF diagram is 32767. When there are more samples, only every
  // sampleSelect-th sample will be plotted
    sampleSelect = (rowCount1 / 32760) +1;

    file.close();
    rowTotal1 = rowCount1 - rowOffset1;

    // Frequency range ui label
    temp = "Frequency range:\n";
    QString freqLo = QString::number(freq.toInt() - span.toInt()*500);
    QString freqHi = QString::number(freq.toInt() + span.toInt()*500);
    QString range  = freqLo + " - " + freqHi + " kHz";
    ui->labelFreqRange->setText(temp + range);


    // Display count of samples
    temp = "No. of samples:\n";
    ui->labelNoOfMeas->setText(temp + QString::number(rowTotal1));

    // Measurement length in hours
    QTime timeStart, timeStop, timeLength;
    timeStart= QTime::fromString(timeList.first(), "hh:mm:ss.zzz");
    timeStop = QTime::fromString(timeList.last(), "hh:mm:ss.zzz");

    int hours   = timeStop.hour()-timeStart.hour();
    int minutes = timeStop.minute()-timeStart.minute();
    int seconds = timeStop.second()-timeStart.second();

    timeLength.setHMS(hours, minutes, seconds, 0);

    temp = "Measurement Length:\n";
    ui->labelMeasLength->setText(temp + timeLength.toString("hh:mm:ss"));

    // time of start&end of measurement display
    temp = "Duration:\n";
    QString kr1 = timeList.first();
    QString kr2 = timeList.last();

//    QString duration = timeList.first()+ " - " + timeList.last();
    QString duration = kr1.left(8) + " - " + kr2.left(8);
    ui->labelDuration->setText(temp + duration);


    // set x axis keys
    colorMap1->data()->setKeyRange(QCPRange(freqLo.toDouble(), freqHi.toDouble()));

    // set y axis keys
    colorMap1->data()->setValueSize(rowTotal1);
    colorMap1->data()->setValueRange(QCPRange(dataTime1.first(), dataTime1.at(rowTotal1)));


    noOfRow = rowTotal1;

    // allowing the GUI elements
    ui->SliderTime->setRange(1, noOfRow-2);
    ui->SliderTime->setValue(0);
    ui->LEtime->setText(timeList.at(0));
    ui->SliderTime->setDisabled(false);
    timeLine->setVisible(true);
    ui->SBtreshold->setDisabled(false);
    ui->SliderTreshold->setDisabled(false);

    PBplotData_clicked();
}

void LoadData::PBplotData_clicked()
// This function plots WF diagram
{
    rowCount1 = 1;      // counter of lines in the whole file (including header)
    QFile file(filename1);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){     // warning if file not open
        qDebug() << "File not opened correctly!";
        QMessageBox::warning(this, "Error", "File not open!");
        return;
    }
    file.seek(0);

    QString line;
    QStringList frequenz;

    QTextStream stream(&file);
    while (stream.readLineInto(&line)){     // reading the file line by line
        rowCount1++;    // increment line counter

        if ((rowCount1 > rowOffset1) && (rowCount1 < noOfRow+rowOffset1)){
            frequenz = line.split('\t');


            // reducing number of samples plotted in WF in case there are over 32760 samples
            if(rowCount1%sampleSelect == 0){
                for (int m=1; m<noOfCol; m++){
                    QString pomoc = frequenz.at(m);

                    if(m<int(noOfCol/2)) // swapping the left and right halves of spectrum
                        colorMap1->data()->setCell(m+noOfCol/2-1, rowCount1-rowOffset1, pomoc.toDouble());
                    else
                        colorMap1->data()->setCell(m-noOfCol/2, rowCount1-rowOffset1, pomoc.toDouble());
                }
            }
        }
    }
    file.close();

    double WFupper = -20;
    colorMap1->setDataRange(QCPRange(-100, WFupper));

    ui->SBupperWF->setValue(WFupper);

    ui->waterfallWidget->rescaleAxes();
    ui->waterfallWidget->replot();

    plotSpectrum(1);
}

void LoadData::plotSpectrum(int idx){
    int busy = 0;
    QVector<double> powValue, freqValue;
    for (int i=0; i<noOfCol-2; i++){
        freqValue.append(freqList.at(i).toDouble());    // keys
        powValue.append(colorMap1->data()->cell(i, idx));     // values

        if (colorMap1->data()->cell(i+1, idx) > ui->SBtreshold->value()){
            usage1->data()->setCell(i, 1, 0);
            busy++;
        }
        else
             usage1->data()->setCell(i, 1, 1);
    }

    ui->graphWidget->graph(0)->setData(freqValue, powValue);
    ui->graphWidget->yAxis->setRange(ui->SBlowerSpectrum->value(), ui->SBupperSpectrum->value());
    ui->graphWidget->xAxis->setRange(freqValue.first(), freqValue.last());

    tresholdLine1->start->setCoords(freqValue.first(), ui->SBtreshold->value());
    tresholdLine1->end->setCoords(freqValue.last(), ui->SBtreshold->value());
    ui->LEoccupancy->setText(QString::number(double (100*busy)/noOfCol, 'f', 1));


    timeLine->start->setCoords(colorMap1->data()->keyRange().lower, dataTime1.at(idx));
    timeLine->end->setCoords(colorMap1->data()->keyRange().upper, dataTime1.at(idx));


    ui->bandUsage->replot();
    ui->graphWidget->replot();
    ui->waterfallWidget->replot();
}

void LoadData::plotOccupancy(){
    // comparison of treshold (in UI) with a single sweep (selected by SliderTime). If measured value croses the treshold, 1 is assigned to the plot and vice versa
    // in the end BW occupancy is calculated and printed into UI
    int busy = 0;

    for (int i=0; i<noOfCol-2; i++){
        if (colorMap1->data()->cell(i, ui->SliderTime->value()) > ui->SBtreshold->value()){
            usage1->data()->setCell(i, 2, 1);
            busy++;
        }
        else
            usage1->data()->setCell(i, 2, 0);
    }

    tresholdLine1->start->setCoords(colorMap1->data()->keyRange().lower, ui->SBtreshold->value());      // dataFreq1.first().toDouble()
    tresholdLine1->end->setCoords(colorMap1->data()->keyRange().upper, ui->SBtreshold->value());      // dataFreq1.last().toDouble()

    ui->LEoccupancy->setText(QString::number(double (100*busy)/noOfCol, 'f', 1));
    ui->bandUsage->replot();
}

void LoadData::WFdataRange(int lower, int upper)
{
    colorMap1->setDataRange(QCPRange(lower, upper));
    ui->waterfallWidget->replot();
}

void LoadData::on_SliderTime_valueChanged(int value)
{
    QString label = timeList.at(value);
    ui->LEtime->setText(label);
    plotSpectrum(value);

}

void LoadData::on_SBupperSpectrum_valueChanged(int arg1)
{
    ui->graphWidget->yAxis->setRangeUpper(arg1);
    ui->graphWidget->replot();
}

void LoadData::on_SBlowerSpectrum_valueChanged(int arg1)
{
    ui->graphWidget->yAxis->setRangeLower(arg1);
    ui->graphWidget->replot();
}

void LoadData::on_SBtreshold_valueChanged(int arg1)
{
    plotSpectrum(ui->SliderTime->value());
}

void LoadData::on_checkBoxLineVisible_toggled(bool checked)
{
    tresholdLine1->setVisible(checked);
    ui->graphWidget->replot();
}

void LoadData::spectrumRangeChanged(QCPRange newRange){
    // change values of spinboxes
    ui->SBlowerSpectrum->setValue(newRange.lower);
    ui->SBupperSpectrum->setValue(newRange.upper);
}


void LoadData::on_PBrescaleSpectrum_clicked()
{
    ui->graphWidget->rescaleAxes();
    if(ui->graphWidget->yAxis->range().upper< -60)
        ui->graphWidget->yAxis->setRangeUpper(-60);
    ui->graphWidget->replot();
}

void LoadData::on_SBupperWF_valueChanged(int arg1)
{
    WFdataRange(ui->SBlowerWF->value(), arg1);
}

void LoadData::on_SBlowerWF_valueChanged(int arg1)
{
    WFdataRange( arg1, ui->SBupperWF->value());
}

void LoadData::WFupdateSBRange(QCPRange newRange){
    ui->SBlowerWF->setValue(int(newRange.lower));
    ui->SBupperWF->setValue(int(newRange.upper));
}


void LoadData::on_PBrescaleWF_clicked()
{
    ui->waterfallWidget->rescaleAxes();
    colorMap1->rescaleDataRange();
    ui->waterfallWidget->replot();
}
