/* Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file      */
//===----------------------------------------------------------------------===//
///
/// \file daqlite.cpp
///
/// \brief key primitive - Kafka consume happens here
///
///
//===----------------------------------------------------------------------===//

#include <Configuration.h>
#include <Custom2DPlot.h>
#include <MainWindow.h>
#include <WorkerThread.h>

#include <QApplication>
#include <QPlot/QPlot.h>
#include <fmt/format.h>
#include <QCommandLineParser>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  QCommandLineParser CLI;
  CLI.setApplicationDescription("daqlite: when you're' driving home");
  CLI.addHelpOption();

  QCommandLineOption fileOption("f", "Configuration file", "file");
  CLI.addOption(fileOption);
  CLI.process(app);

  Configuration Config;
  if (CLI.isSet(fileOption)) {
    std::string FileName = CLI.value(fileOption).toStdString();
    fmt::print("Getting config from file {}\n", FileName);
    Config.fromJsonFile(FileName);
    Config.print();
  }

  MainWindow DaquiriLite(Config);

  return app.exec();
}
