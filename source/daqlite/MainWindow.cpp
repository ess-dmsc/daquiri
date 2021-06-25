/* Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file      */
//===----------------------------------------------------------------------===//
///
/// \file MainWindow.cpp
///
//===----------------------------------------------------------------------===//

#include <MainWindow.h>
#include "ui_MainWindow.h"
#include <string.h>

MainWindow::MainWindow(Configuration &Config, QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , mConfig(Config) {

  ui->setupUi(this);

  if (strcmp(Config.Plot.PlotType.c_str(), "tof") == 0) {
    TOF = true;
  }

  if (!TOF) {
    // Always create the XY plot
    Plot2DXY = new Custom2DPlot(mConfig, 0);
    ui->gridLayout->addWidget(Plot2DXY, 0, 0, 1, 1);
    // If detector is 3D, also create XZ and YZ
    if (Config.Geometry.ZDim > 1) {
      Plot2DXZ = new Custom2DPlot(mConfig, 1);
      ui->gridLayout->addWidget(Plot2DXZ, 0, 1, 1, 1);
      Plot2DYZ = new Custom2DPlot(mConfig, 2);
      ui->gridLayout->addWidget(Plot2DYZ, 0, 2, 1, 1);
    }
  } else {
    PlotTOF = new CustomTofPlot(mConfig);
    ui->gridLayout->addWidget(PlotTOF, 0, 0, 1, 1);
  }

  ui->lblDescriptionText->setText(mConfig.Plot.PlotTitle.c_str());
  ui->lblEventRateText->setText("0");

  connect(ui->pushButtonQuit, SIGNAL(clicked()), this, SLOT(handleExitButton()));
  connect(ui->pushButtonClear, SIGNAL(clicked()), this, SLOT(handleClearButton()));
  connect(ui->pushButtonLog, SIGNAL(clicked()), this, SLOT(handleLogButton()));
  connect(ui->pushButtonGradient, SIGNAL(clicked()), this, SLOT(handleGradientButton()));
  connect(ui->pushButtonInvert, SIGNAL(clicked()), this, SLOT(handleInvertButton()));

  updateGradientLabel();
  show();
  startKafkaConsumerThread();
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::startKafkaConsumerThread() {
  KafkaConsumerThread = new WorkerThread(mConfig);
  qRegisterMetaType<int>("uint64_t&");
  connect(KafkaConsumerThread, &WorkerThread::resultReady, this,
          &MainWindow::handleKafkaData);
  KafkaConsumerThread->start();
}

// SLOT
void MainWindow::handleKafkaData(int EventRate) {
  ui->lblEventRateText->setText(QString::number(EventRate));
  KafkaConsumerThread->mutex.lock();
  if (!TOF) {
    Plot2DXY->addData(KafkaConsumerThread->consumer()->mHistogramPlot);
    if (mConfig.Geometry.ZDim > 1) {
      Plot2DXZ->addData(KafkaConsumerThread->consumer()->mHistogramPlot);
      Plot2DYZ->addData(KafkaConsumerThread->consumer()->mHistogramPlot);
    }
  } else {
    PlotTOF->addData(KafkaConsumerThread->consumer()->mHistogramTofPlot);
  }
  KafkaConsumerThread->mutex.unlock();
}

// SLOT
void MainWindow::handleExitButton() { QApplication::quit(); }

void MainWindow::handleClearButton() {
  if (!TOF) {
    Plot2DXY->clearDetectorImage();
    if (mConfig.Geometry.ZDim > 1) {
      Plot2DXZ->clearDetectorImage();
      Plot2DYZ->clearDetectorImage();
    }
  } else {
    PlotTOF->clearDetectorImage();
  }
}

void MainWindow::updateGradientLabel() {
  if (TOF)
    return;

  if (mConfig.Plot.InvertGradient)
    ui->lblGradientText->setText(QString::fromStdString(mConfig.Plot.ColorGradient + " (I)"));
  else
    ui->lblGradientText->setText(QString::fromStdString(mConfig.Plot.ColorGradient));

}

// toggle the log scale flag
void MainWindow::handleLogButton() {
  mConfig.Plot.LogScale = not mConfig.Plot.LogScale;
}

// toggle the invert gradient flag
void MainWindow::handleInvertButton() {
  if (TOF)
    return;

  mConfig.Plot.InvertGradient = not mConfig.Plot.InvertGradient;
  updateGradientLabel();
}

void MainWindow::handleGradientButton() {
  if (TOF)
    return;

  mConfig.Plot.ColorGradient = Plot2DXY->getNextColorGradient(mConfig.Plot.ColorGradient);
  updateGradientLabel();
}
