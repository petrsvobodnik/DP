#include "loaddata.h"
#include "ui_loaddata.h"
#include "mainwindow.h"

QString filename1, dateOfMeas1;
int rowCount1 = 0, columnCount1 = 0, rowTotal1;
int rowOffset1 = 6, columnOffset1 = 1;
int noOfRow =100, noOfCol = 1024;      \

QStringList freqList, timeList;

QVector<double> dataFreq1;      //
QVector<double> dataTime1;

QCPColorMap *colorMap1 = NULL;


void printMap(QCPColorMap* map){
    // function for printing out the values of ColorMapData to console
    for (int radky=0; radky < noOfRow; radky++){
        for (int sloupce=0; sloupce < noOfCol; sloupce++)
            printf("%2.2f ",map->data()->cell(sloupce, radky));

        printf("\n");
    }
    printf("\n");
}


LoadData::LoadData(QWidget *parent) :       // GUI constructor
    QDialog(parent),
    ui(new Ui::LoadData)
{
    ui->setupUi(this);

    ui->LEfilename->setReadOnly(true);

    ui->graphWidget->addGraph();
    ui->graphWidget->graph(0)->setName("FFT diagram");
    ui->graphWidget->yAxis->setRange(-100, 0);
    ui->graphWidget->yAxis->setLabel("Power [dBm]");
    ui->graphWidget->xAxis->setLabel("Frequency [kHz]");
    ui->graphWidget->setInteractions(QCP::iRangeZoom | QCP::iSelectPlottables| QCP::iRangeDrag);
    ui->graphWidget->axisRect()->setRangeDrag(Qt::Horizontal);
    ui->graphWidget->axisRect()->setRangeZoom(Qt::Horizontal);


    ui->waterfallWidget->setInteractions(QCP::iRangeZoom | QCP::iSelectPlottables| QCP::iRangeDrag);
    ui->waterfallWidget->axisRect()->setRangeDrag( Qt::Vertical);
    ui->waterfallWidget->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    colorMap1 = new QCPColorMap(ui->waterfallWidget->xAxis, ui->waterfallWidget->yAxis);
    colorMap1->data()->setSize(noOfCol, noOfRow);
//    colorMap1->setGradient(QCPColorGradient::gpPolar);

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

    ui->LEupperWF->setReadOnly(true);

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

        default:        // saves times of measurement in 'rowTotal1' iterations
            timeList.append(frequenz.at(0));
            dataTime1.append((QDateTime::fromString(frequenz.at(0)+" "+dateOfMeas1, "hh:mm:ss.zzz dd. MM. yyyy")).toTime_t());
            break;
        }

        rowCount1++;
    }

    file.close();

    rowTotal1 = rowCount1 - rowOffset1;
    ui->SBsamplesShown->setMaximum(rowTotal1);  // SpinBox displaying total number of measurements
    ui->SBsamplesShown->setDisabled(false);
    ui->SBsamplesShown->setValue(rowTotal1);

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

    ui->SliderTime->setRange((int) dataTime1.first(), (int) dataTime1.at(rowTotal1));


    noOfRow = rowTotal1;
//    ui->SliderTime->setMaximum(noOfRow);
    on_PBplotData_clicked();
}

void LoadData::on_PBplotData_clicked()
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

        if ((rowCount1 > rowOffset1) && (rowCount1 < noOfRow+rowOffset1)){       // for testing purposes
            frequenz = line.split('\t');

            for (int m=1; m<noOfCol; m++){
                QString pomoc = frequenz.at(m+1);
                colorMap1->data()->setCell(m, rowCount1-rowOffset1, pomoc.toDouble());
            }
//        dataFreq1.append(valueFreq.toDouble());
        }
    }
    file.close();

//    Color range manipulation
    colorMap1->rescaleDataRange();
    double range2 =  colorMap1->dataRange().upper;
    int changedRange = colorMap1->dataRange().lower;
    ui->LEupperWF->setText(QString::number(range2));
    ui->SliderColorScale->setValue(int (range2));


    ui->waterfallWidget->rescaleAxes();
    ui->waterfallWidget->replot();

    plotSpectrum(colorMap1, 8);
//    printMap(colorMap1);
}

void LoadData::plotSpectrum(QCPColorMap *map, int idx){
    QVector<double> powValue, freqValue;
    for (int i=0; i<noOfCol; i++){
        powValue.append(map->data()->cell(i, idx));     // values
        freqValue.append(freqList.at(i).toDouble());    // keys
    }

    ui->graphWidget->graph(0)->setData(freqValue, powValue);
    ui->graphWidget->rescaleAxes();
    ui->graphWidget->xAxis->setRange(freqValue.first(), freqValue.last());
    ui->graphWidget->replot();

}

void LoadData::on_SliderColorScale_valueChanged(int value)
{
    colorMap1->setDataRange(QCPRange(-100, value));
    ui->waterfallWidget->replot();
    ui->LEupperWF->setText(QString::number(value));
}

void LoadData::on_SliderTime_valueChanged(int value)
{
    double x,y;
    colorMap1->data()->cellToCoord(1, value, &x, &y);
    QDateTime time;
    time.addSecs(int64_t (value));
//    qDebug() << "x: " << x << "\ny:" << y;
//    QDateTime::currentDateTime().toString("hh:mm:ss.zzz")
    qDebug() << time.toString("hh:mm:ss");
    ui->LEtime->setText(time.toString("hh:mm:ss"));
}
