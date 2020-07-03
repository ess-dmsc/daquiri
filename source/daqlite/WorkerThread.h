
#pragma once
#include <ESSConsumer.h>
#include <QThread>
#include <iostream>

class WorkerThread : public QThread {
  Q_OBJECT

public:
  WorkerThread(QObject *parent, Configuration &Config) : mConfig(Config) {
    Consumer = new ESSConsumer(Config);
  };
  ~WorkerThread(){};

  void run() override;

  ESSConsumer *Consumer;

signals:
  void resultReady(int &val);

private:
  Configuration &mConfig;

};
