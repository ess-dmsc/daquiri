#include <MainWindow.h>
#include "ui_MainWindow.h"

MainWindow::MainWindow(Configuration &Config, QWidget *parent)
    : mConfig(Config)
    , QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    Plot2D = new Custom2DPlot(mConfig);
    setupLayout();
    show();
    startKafkaConsumerThread();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupLayout() {
  setWindowTitle("Daquiri lite");

  ui->labelDescription->setText(mConfig.Plot.Title.c_str());
  ui->labelEventRate->setText("0");

  ui->gridLayout->addWidget(Plot2D, 0, 0);

  connect(ui->pushButtonQuit, SIGNAL(clicked()), this, SLOT(handleExitButton()));
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
  ui->labelEventRate->setText(QString::number(EventRate));
  Plot2D->addData(EventRate, KafkaConsumerThread->consumer()->mHistogramPlot);
}

// SLOT
void MainWindow::handleExitButton() { QApplication::quit(); }
