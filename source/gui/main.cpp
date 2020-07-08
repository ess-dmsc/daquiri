#include <gui/daquiri.h>
#include <consumers/consumers_autoreg.h>
#include <producers/producers_autoreg.h>
#include <QApplication>
#include <QCommandLineParser>
#include <gui/Profiles.h>

int main(int argc, char *argv[])
{
  CustomLogger::initLogger(spdlog::level::info, "daquiri.log");

  QApplication app(argc, argv);
  QApplication::setOrganizationName("ESS");
  QApplication::setApplicationName("daquiri");
//  QApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("daquiri: snapping at the heels of the martini");
  parser.addHelpOption();
//  parser.addVersionOption();

  QCommandLineOption openOption(QStringList() << "o" << "open",
          QApplication::translate("main", "Open project"));
  parser.addOption(openOption);

  QCommandLineOption runOption(QStringList() << "r" << "run",
          QApplication::translate("main", "Run acquisition"));
  parser.addOption(runOption);

  QCommandLineOption profileOption(
        QStringList() << "p" << "profile",
        QApplication::translate("main", "Open <profile>."),
        QApplication::translate("main", "profile"));
  parser.addOption(profileOption);

  parser.process(app);

  bool startnew = parser.isSet(runOption);
  bool opennew = parser.isSet(openOption) || startnew;
  QString profile = parser.value(profileOption);

  if (!profile.isEmpty())
  {
    QSettings settings;
    settings.beginGroup("Program");
    settings.setValue("current_profile", profile);
    settings.setValue("auto_boot", true);
    // \todo this might be overkill?
  }

  if (!parser.isSet("h"))
  {
    hdf5::error::Singleton::instance().auto_print(false);

    producers_autoreg();
    consumers_autoreg();

    daquiri w(0, opennew, startnew);
    w.show();

    return app.exec();
  }
}
