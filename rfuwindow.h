#ifndef RFUWINDOW_H
#define RFUWINDOW_H

#include <QDialog>

namespace Ui {
class RFUwindow;
}

class RFUwindow : public QDialog
{
    Q_OBJECT

public:
    explicit RFUwindow(QWidget *parent = 0);
    ~RFUwindow();

private:
    Ui::RFUwindow *ui;
};

#endif // RFUWINDOW_H
