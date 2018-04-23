#ifndef LOADDATA_H
#define LOADDATA_H

#include <QDialog>
#include <qcustomplot.h>

namespace Ui {
class LoadData;
}

class LoadData : public QDialog
{
    Q_OBJECT

public:
    explicit LoadData(QWidget *parent = 0);
    ~LoadData();

private slots:
    void on_PBopenFile_clicked();
    void on_PBplotData_clicked();
    void on_SliderColorScale_valueChanged(int value);
    void on_SliderTime_valueChanged(int value);
    void on_SBupperRange_valueChanged(int arg1);
    void on_SBlowerRange_valueChanged(int arg1);

private:
    Ui::LoadData *ui;
    void plotSpectrum(QCPColorMap *map, int idx);
};

#endif // LOADDATA_H
