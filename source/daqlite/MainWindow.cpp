#include <MainWindow.h>
#include "ui_MainWindow.h"

MainWindow::MainWindow(Configuration &Config, QWidget *parent)
    : mConfig(Config)
    , QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Plot2D = new Custom2DPlot(mConfig);
    setupLayout();
    show();
    startKafkaConsumerThread();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupLayout() {
  setWindowTitle("Daquiri lite");
  resize(600, 500);

  ui->gridLayout->addWidget(Plot2D, 0, 0);

  connect(ui->pushButtonQuit, SIGNAL(clicked()), this, SLOT(handleExitButton()));
}

void MainWindow::startKafkaConsumerThread() {
  KafkaConsumerThread = new WorkerThread(mConfig);
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
