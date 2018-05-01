#ifndef RFUSETTING_H
#define RFUSETTING_H

#include <QDialog>

namespace Ui {
class RFUsetting;
}

class RFUsetting : public QDialog
{
    Q_OBJECT

public:
    explicit RFUsetting(QWidget *parent = 0);
    ~RFUsetting();


private slots:
    void on_PBok_clicked();
    void on_PBgetGain_clicked();
    void on_PBstopRotator_clicked();
    void on_PBsetRotator_clicked();

    void on_PBrotLEFT_released();
    void on_PBrotRIGHT_released();
    void on_PBrotRIGHT_pressed();
    void on_PBrotLEFT_pressed();
    void computeGain();
    void on_PBconnectAR_clicked();
    void on_PBsetpolarisation_clicked();

    void on_PBconnectAU_clicked();

public slots:
    void RBgroupLEVEL_clicked(int );
    void RBgroupANT_clicked(int );
    void RBgroupFILT_clicked (int);
    void getRotatorState();


private:
    Ui::RFUsetting *ui;
};

#endif // RFUSETTING_H
