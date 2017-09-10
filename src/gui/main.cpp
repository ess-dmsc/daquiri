#include "daquiri.h"
#include "consumers_autoreg.h"
#include "producers_autoreg.h"
#include <QApplication>
#include <QCommandLineParser>
#include "Profiles.h"

int main(int argc, char *argv[])
{
  producers_autoreg();
  QApplication app(argc, argv);
  QApplication::setOrganizationName("ESS");
  QApplication::setApplicationName("daquiri");
  QApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("Test helper");
  parser.addHelpOption();
  parser.addVersionOption();

  // A boolean option with multiple names (-f, --force)
  QCommandLineOption openOption(QStringList() << "o" << "open",
          QApplication::translate("main", "Open project"));
  parser.addOption(openOption);

  // A boolean option with multiple names (-f, --force)
  QCommandLineOption runOption(QStringList() << "r" << "run",
          QApplication::translate("main", "Run acquisition"));
  parser.addOption(runOption);

  // An option with a value
  QCommandLineOption profileOption(
        QStringList() << "p" << "profile",
        QApplication::translate("main", "Open <profile>."),
        QApplication::translate("main", "profile"));
  parser.addOption(profileOption);

  // Process the actual command line arguments given by the user
  parser.process(app);

  const QStringList args = parser.positionalArguments();
  // source is args.at(0), destination is args.at(1)

  bool startnew = parser.isSet(runOption);
  bool opennew = parser.isSet(openOption) || startnew;
  QString profile = parser.value(profileOption);

  if (!profile.isEmpty())
  {
    Profiles::select_profile(profile, true);
  }

  daquiri w(0, opennew, startnew, "");
  w.show();

  return app.exec();
}
