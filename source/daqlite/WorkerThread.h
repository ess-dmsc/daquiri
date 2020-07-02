
#pragma once
#include <ESSConsumer.h>
#include <QThread>
#include <iostream>

class WorkerThread : public QThread
{
  Q_OBJECT

  public:
    WorkerThread(QObject *parent, Configuration & Config)
       : mConfig(Config) {
         Consumer = new ESSConsumer(Config);
       };
    ~WorkerThread() {};

    void run() override;

  signals:
    void resultReady(int & val);

  private:
    Configuration & mConfig;
    ESSConsumer * Consumer;
};
