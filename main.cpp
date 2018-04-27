#include "mainwindow.h"
#include <QApplication>

#define MHz 1000000
#define GHz 1000000000



int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;

    w.setWindowTitle("Frequency spectrum measurement");
    w.show();

    int ret = a.exec();
    return ret;

}
