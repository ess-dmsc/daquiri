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

  bool opennew {false};
  bool startnew {false};
  if (argc > 1)
  {
    startnew = (std::string(argv[1]) == "start");
    opennew = startnew || (std::string(argv[1]) == "open");
  }

  QCoreApplication::setOrganizationName("ESS");
  QCoreApplication::setApplicationName("daquiri");

  daquiri w(0, opennew, startnew);
  w.show();

  return a.exec();
}
