#include "daquiri.h"
#include <QApplication>

#include <QSettings>
#include <QDir>
#include "json_file.h"
#include "custom_logger.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("ESS");
    QCoreApplication::setApplicationName("daquiri");

    daquiri w;
    w.show();

    return a.exec();
}
