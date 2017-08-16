#include "daquiri.h"
#include <QApplication>

#include <QSettings>
#include <QDir>
#include "json_file.h"
#include "custom_logger.h"

#include "consumers_autoreg.h"
#include "producers_autoreg.h"

int main(int argc, char *argv[])
{
  producers_autoreg();
  QApplication a(argc, argv);

  QCoreApplication::setOrganizationName("ESS");
  QCoreApplication::setApplicationName("daquiri");

  daquiri w;
  w.show();

  return a.exec();
}
