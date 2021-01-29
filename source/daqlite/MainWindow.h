/* Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file      */
//===----------------------------------------------------------------------===//
///
/// \file MainWindow.h
///
/// Main (and only) window for daqlite
//===----------------------------------------------------------------------===//

#pragma once

#include <Configuration.h>
#include <Custom2DPlot.h>
#include <WorkerThread.h>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
  MainWindow(Configuration &Config, QWidget *parent = nullptr);
  ~MainWindow();

  /// \brief spin up a thread for consuming topic
  void startKafkaConsumerThread();

public slots:
  void handleExitButton();
  void handleClearButton();
  void handleKafkaData(int EventRate);

private:
  Ui::MainWindow *ui;

  /// \brief
  Custom2DPlot *Plot2DXY; // xy plots
  Custom2DPlot *Plot2DXZ; // xz plots
  Custom2DPlot *Plot2DYZ; // yz plots

  /// \brief configuration obtained from main()
  Configuration mConfig;

  /// \brief
  WorkerThread *KafkaConsumerThread;
};
