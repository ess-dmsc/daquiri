#include "daquiri.h"
#include <QApplication>
#include <vector>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("ESS");
    QCoreApplication::setApplicationName("daquiri");

    daquiri w;
    w.show();

    return a.exec();
}
