/* Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file      */
//===----------------------------------------------------------------------===//
///
/// \file MainWindow.h
///
/// \brief Defines the Qt GUI
///
/// Also links together plotting, consumer thread and config data
//===----------------------------------------------------------------------===//

#pragma once

#include <Configuration.h>
#include <Custom2DPlot.h>
#include <QMainWindow>
#include <WorkerThread.h>

class MainWindow : public QWidget {
  Q_OBJECT

public:
  MainWindow(Configuration &Config);

  // Create GUI layout
  void setupLayout();

  /// \brief spin up a thread for consuming topic
  void startKafkaConsumerThread();

public slots:
  void handleExitButton();
  void handleKafkaData(int i);

private:
  /// \brief
  Custom2DPlot *Plot2D;

  /// \brief configuration obtained from main()
  Configuration mConfig;

  /// \brief
  WorkerThread *KafkaConsumerThread;
};
