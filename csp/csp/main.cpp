#include "csp.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Csp w;
    w.show();
    return a.exec();
}
