#ifndef ZEROSPAN_H
#define ZEROSPAN_H

#include <QDialog>
#include <QWidget>

namespace Ui {
class zeroSpan;
}

class zeroSpan : public QDialog
{
    Q_OBJECT

public:
    explicit zeroSpan(QWidget *parent = 0);
    ~zeroSpan();

private slots:
    void on_PBload_clicked();
    
    void on_PBplot_clicked();

    void on_SBupperRange_valueChanged(int arg1);

    void on_SBlowerRange_valueChanged(int arg1);

    void on_CBtime_currentIndexChanged(int index);

private:
    Ui::zeroSpan *ui;
};

#endif // ZEROSPAN_H
