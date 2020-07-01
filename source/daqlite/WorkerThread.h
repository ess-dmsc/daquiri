
#pragma once
#include <ESSConsumer.h>
#include <QThread>
#include <iostream>

class WorkerThread : public QThread
{
  Q_OBJECT

  public:
    WorkerThread(QObject *parent = nullptr) {};
    ~WorkerThread() {};

    void run() override;

  signals:
    void resultReady(int & val);

  private:
    ESSConsumer Consumer;
};
