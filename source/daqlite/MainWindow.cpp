/* Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file      */
//===----------------------------------------------------------------------===//
///
/// \file MainWindow.cpp
///
//===----------------------------------------------------------------------===//

#include <MainWindow.h>
#include <QWidget>

MainWindow::MainWindow(Configuration &Config) : mConfig(Config) {
  Plot2D = new Custom2DPlot(mConfig);
  setupLayout();
  show();
  startKafkaConsumerThread();
}

void MainWindow::setupLayout() {
  setWindowTitle("Daquiri lite");
  resize(600, 500);

  // main vertical layout
  QVBoxLayout *pMainLayout = new QVBoxLayout(this);
  setLayout(pMainLayout);

  // first sub layouts
  QHBoxLayout *pTopHBox = new QHBoxLayout;
  pMainLayout->addLayout(pTopHBox);

  QHBoxLayout *pBtnHBox = new QHBoxLayout;
  pMainLayout->addLayout(pBtnHBox);

  // create the group box
  QGroupBox *plotBox = new QGroupBox(this);
  plotBox->setTitle(mConfig.Plot.Title.c_str());

  // create the layout for the plotBox
  QGridLayout *plotLayout = new QGridLayout;

  plotLayout->addWidget(Plot2D, 0, 0);
  if (mConfig.Geometry.ZDim > 1) {
    auto plot2 = new Custom2DPlot(mConfig);
    plotLayout->addWidget(plot2, 0, 1);
  }
  plotBox->setLayout(plotLayout);
  pTopHBox->addWidget(plotBox);

  // create the button area
  QPushButton *pBtn = new QPushButton(QObject::tr("Quit"), this);
  pBtnHBox->addWidget(pBtn);
  pBtnHBox->addItem(
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

  connect(pBtn, SIGNAL(clicked()), this, SLOT(handleExitButton()));
}

void MainWindow::startKafkaConsumerThread() {
  KafkaConsumerThread = new WorkerThread(this, mConfig);
  qRegisterMetaType<int>("int&");
  connect(KafkaConsumerThread, &WorkerThread::resultReady, this,
          &MainWindow::handleKafkaData);
  KafkaConsumerThread->start();
}

// SLOT
void MainWindow::handleKafkaData(int i) {
  Plot2D->addData(i, KafkaConsumerThread->consumer()->mHistogramPlot);
}

// SLOT
void MainWindow::handleExitButton() { QApplication::quit(); }
