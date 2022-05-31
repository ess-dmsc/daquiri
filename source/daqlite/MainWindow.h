// Copyright (C) 2020 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file MainWindow.h
///
/// Main (and only) window for daqlite
//===----------------------------------------------------------------------===//

#pragma once

#include <Configuration.h>
#include <Custom2DPlot.h>
#include <CustomAMOR2DTOFPlot.h>
#include <CustomTofPlot.h>
#include <QMainWindow>
#include <WorkerThread.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(Configuration &Config, QWidget *parent = nullptr);
  ~MainWindow();

  /// \brief create the plot widgets
  void setupPlots();

  /// \brief spin up a thread for consuming topic
  void startKafkaConsumerThread();

  /// \brief update GUI label text
  void updateGradientLabel();

  /// \brief update GUI label text
  void updateAutoScaleLabels();

public slots:
  void handleExitButton();
  void handleClearButton();
  void handleLogButton();
  void handleGradientButton();
  void handleInvertButton();
  void handleAutoScaleXButton();
  void handleAutoScaleYButton();
  void handleKafkaData(int ElapsedCountNS);

private:
  Ui::MainWindow *ui;
  enum PlotType {none, pixels, tof, tof2d};
  PlotType plottype{none};

  CustomAMOR2DTOFPlot *PlotTOF2D; /// Experimental
  Custom2DPlot *Plot2DXY; // xy plots
  Custom2DPlot *Plot2DXZ; // xz plots
  Custom2DPlot *Plot2DYZ; // yz plots
  CustomTofPlot *PlotTOF;

  /// \brief configuration obtained from main()
  Configuration mConfig;

  /// \brief
  WorkerThread *KafkaConsumerThread;

};
