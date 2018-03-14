#ifndef FREQSETTING_H
#define FREQSETTING_H

#include <QDialog>
#include "hackrf.h"
//#include "mainwindow.h"

namespace Ui {
class freqSetting;
}

class freqSetting : public QDialog
{
    Q_OBJECT

public:
    explicit freqSetting(QWidget *parent = 0);
    ~freqSetting();

private slots:
    void spinSlidVGA(int);
    void slidSpinVGA(int);
    void spinSlidLNA(int);
    void slidSpinLNA(int);


    void on_PBcancel_clicked();
    void on_PBapply_clicked();
    void on_PBok_clicked();

private:
    Ui::freqSetting *ui;
    hackrf_device *sdr;
};

#endif // FREQSETTING_H
