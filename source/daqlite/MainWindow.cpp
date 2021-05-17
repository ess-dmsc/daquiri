/* Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file      */
//===----------------------------------------------------------------------===//
///
/// \file MainWindow.cpp
///
//===----------------------------------------------------------------------===//

#include <MainWindow.h>
#include "ui_MainWindow.h"

MainWindow::MainWindow(Configuration &Config, QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , mConfig(Config) {

  ui->setupUi(this);
  setWindowTitle("Daquiri lite");

  // printf("MainWindow object name %s\n", qPrintable(this->objectName()));

  // Always create the XY plot
  Plot2DXY = new Custom2DPlot(mConfig, 0);
  ui->gridLayout->addWidget(Plot2DXY, 0, 0, 1, 1);

  printf("Plot2DXY: width %d, height %d\n", Plot2DXY->width(), Plot2DXY->height());


  // If detector is 3D, also create XZ and YZ
  if (Config.Geometry.ZDim > 1) {
    Plot2DXZ = new Custom2DPlot(mConfig, 1);
    ui->gridLayout->addWidget(Plot2DXZ, 0, 1, 1, 1);
    Plot2DYZ = new Custom2DPlot(mConfig, 2);
    ui->gridLayout->addWidget(Plot2DYZ, 0, 2, 1, 1);
  }

  ui->lblDescriptionText->setText(mConfig.Plot.Title.c_str());
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
  Plot2DXY->addData(EventRate, KafkaConsumerThread->consumer()->mHistogramPlot);
  if (mConfig.Geometry.ZDim > 1) {
    Plot2DXZ->addData(EventRate, KafkaConsumerThread->consumer()->mHistogramPlot);
    Plot2DYZ->addData(EventRate, KafkaConsumerThread->consumer()->mHistogramPlot);
  }
}

// SLOT
void MainWindow::handleExitButton() { QApplication::quit(); }

void MainWindow::handleClearButton() {
  Plot2DXY->clearDetectorImage();
  if (mConfig.Geometry.ZDim > 1) {
    Plot2DXZ->clearDetectorImage();
    Plot2DYZ->clearDetectorImage();
  }
}

void MainWindow::updateGradientLabel() {
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
  mConfig.Plot.InvertGradient = not mConfig.Plot.InvertGradient;
  updateGradientLabel();
}

void MainWindow::handleGradientButton() {
  mConfig.Plot.ColorGradient = Plot2DXY->getNextColorGradient(mConfig.Plot.ColorGradient);
  updateGradientLabel();
}
