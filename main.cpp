#include "laserMarking.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    laserMarking w;
    w.show();
    return a.exec();
}
