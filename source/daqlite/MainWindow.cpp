// Copyright (C) 2020 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file MainWindow.cpp
///
//===----------------------------------------------------------------------===//

#include "ui_MainWindow.h"
#include <MainWindow.h>
#include <string.h>

MainWindow::MainWindow(Configuration &Config, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), mConfig(Config) {

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

  connect(ui->pushButtonQuit, SIGNAL(clicked()), this,
          SLOT(handleExitButton()));
  connect(ui->pushButtonClear, SIGNAL(clicked()), this,
          SLOT(handleClearButton()));
  connect(ui->pushButtonLog, SIGNAL(clicked()), this, SLOT(handleLogButton()));
  connect(ui->pushButtonGradient, SIGNAL(clicked()), this,
          SLOT(handleGradientButton()));
  connect(ui->pushButtonInvert, SIGNAL(clicked()), this,
          SLOT(handleInvertButton()));
  connect(ui->pushButtonAutoScaleX, SIGNAL(clicked()), this,
          SLOT(handleAutoScaleXButton()));
  connect(ui->pushButtonAutoScaleY, SIGNAL(clicked()), this,
          SLOT(handleAutoScaleYButton()));

  updateGradientLabel();
  updateAutoScaleLabels();
  show();
  startKafkaConsumerThread();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::startKafkaConsumerThread() {
  KafkaConsumerThread = new WorkerThread(mConfig);
  qRegisterMetaType<int>("int&");
  connect(KafkaConsumerThread, &WorkerThread::resultReady, this,
          &MainWindow::handleKafkaData);
  KafkaConsumerThread->start();
}

// SLOT
void MainWindow::handleKafkaData(int ElapsedCountMS) {
  auto Consumer = KafkaConsumerThread->consumer();

  uint64_t EventRate = Consumer->EventCount * 1000ULL/ElapsedCountMS;
  uint64_t EventAccept = Consumer->EventAccept * 1000ULL/ElapsedCountMS;
  uint64_t EventDiscardRate = Consumer->EventDiscard * 1000ULL/ElapsedCountMS;


  ui->lblEventRateText->setText(QString::number(EventRate));
  ui->lblAcceptRateText->setText(QString::number(EventAccept));
  ui->lblDiscardedPixelsText->setText(QString::number(EventDiscardRate));

  KafkaConsumerThread->mutex.lock();
  if (!TOF) {
    Plot2DXY->addData(Consumer->mHistogramPlot);
    if (mConfig.Geometry.ZDim > 1) {
      Plot2DXZ->addData(Consumer->mHistogramPlot);
      Plot2DYZ->addData(Consumer->mHistogramPlot);
    }
  } else {
    PlotTOF->addData(Consumer->mHistogramTofPlot);
  }
  Consumer->EventCount = 0;
  Consumer->EventAccept = 0;
  Consumer->EventDiscard = 0;
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
  // Gradient button and lables are not relevant for TOF so we hide them
  if (TOF) {
    ui->pushButtonGradient->setVisible(false);
    ui->pushButtonInvert->setVisible(false);
    ui->lblGradientText->setVisible(false);
    ui->lblGradient->setVisible(false);
    return;
  }

  if (mConfig.Plot.InvertGradient)
    ui->lblGradientText->setText(
        QString::fromStdString(mConfig.Plot.ColorGradient + " (I)"));
  else
    ui->lblGradientText->setText(
        QString::fromStdString(mConfig.Plot.ColorGradient));
}

void MainWindow::updateAutoScaleLabels() {
  // AutoScale button and lables are not relevant for TOF so we hide them
  if (not TOF) {
    ui->pushButtonAutoScaleX->setVisible(false);
    ui->lblAutoScaleXText->setVisible(false);
    ui->lblAutoScaleX->setVisible(false);
    ui->pushButtonAutoScaleY->setVisible(false);
    ui->lblAutoScaleYText->setVisible(false);
    ui->lblAutoScaleY->setVisible(false);
    return;
  }

  if (mConfig.TOF.AutoScaleX)
    ui->lblAutoScaleXText->setText(QString::fromStdString("on"));
  else
    ui->lblAutoScaleXText->setText(QString::fromStdString("off"));

  if (mConfig.TOF.AutoScaleY)
    ui->lblAutoScaleYText->setText(QString::fromStdString("on"));
  else
    ui->lblAutoScaleYText->setText(QString::fromStdString("off"));
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

// toggle the auto scale x button
void MainWindow::handleAutoScaleXButton() {
  if (not TOF)
    return;

  mConfig.TOF.AutoScaleX = not mConfig.TOF.AutoScaleX;
  updateAutoScaleLabels();
}

// toggle the auto scale y button
void MainWindow::handleAutoScaleYButton() {
  if (not TOF)
    return;

  mConfig.TOF.AutoScaleY = not mConfig.TOF.AutoScaleY;
  updateAutoScaleLabels();
}

void MainWindow::handleGradientButton() {
  if (TOF) {
    return;
  }

  mConfig.Plot.ColorGradient =
      Plot2DXY->getNextColorGradient(mConfig.Plot.ColorGradient);
  updateGradientLabel();
}
