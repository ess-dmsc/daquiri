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

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  /// \todo make configs configurable ;-)
  Configuration Config(256, 256, "NMX_detector", "172.17.5.38:9092");
  //Config.Plot.Interpolate = false;
  Config.Plot.Title = fmt::format("{} @ {}  ({} x {})",
         Config.Kafka.Topic, Config.Kafka.Broker,
         Config.Geometry.XDim, Config.Geometry.YDim);

  MainWindow DaquiriLite(Config);

  return app.exec();
}
