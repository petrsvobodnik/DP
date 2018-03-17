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

    void on_PBapply_clicked();

private:
    Ui::RFUsetting *ui;
};

#endif // RFUSETTING_H
