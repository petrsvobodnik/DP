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

private:
    Ui::zeroSpan *ui;
};

#endif // ZEROSPAN_H
