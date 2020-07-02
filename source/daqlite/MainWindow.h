
#pragma once

#include <Configuration.h>
#include <Custom2DPlot.h>
#include <QMainWindow>
#include <WorkerThread.h>

class MainWindow : public QWidget {
  Q_OBJECT

public:
  MainWindow(Configuration &Config);

  // Add widgets
  void setupLayout();

  void startKafkaConsumer();

public slots:
  void handleExitButton();
  void handleKafkaData(int i);

public:
  Configuration mConfig;
  Custom2DPlot *Plot2D; // Detector image - eventually
  WorkerThread *KafkaConsumer;
};
