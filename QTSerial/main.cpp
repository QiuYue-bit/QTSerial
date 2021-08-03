#include "mainwindow.h"
#include <QApplication>


MainWindow* mainWindow;//主窗口的指针
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    mainWindow = &w;

    w.show();

    return a.exec();
}
