#pragma once

#include <QThread>
#include <QMutex>
#include "project.h"
//#include "custom_logger.h"

class ThreadPlotSignal : public QThread
{
  Q_OBJECT
public:
  explicit ThreadPlotSignal(QObject *parent = 0)
    : QThread(parent)
    , terminating_(false)
    , wait_s_(3)
  {}

  void monitor_source(DAQuiri::ProjectPtr pj)
  {
    QMutexLocker locker(&mutex_);
    terminate_helper();
    project_ = pj;
    terminating_.store(false);
    if (!isRunning())
      start(HighPriority);
  }

  void terminate_wait()
  {
    QMutexLocker locker(&mutex_);
    terminate_helper();
  }

  void set_wait_time(uint16_t time)
  {
    wait_s_.store(time);
  }

  DAQuiri::ProjectPtr current_source()
  {
    QMutexLocker locker(&mutex_);
    return project_;
  }

signals:
  void plot_ready();

protected:
  void run() {
//    DBG << "<ThreadPlot> loop starting "
//        << terminating_.load() << " "
//        << (project_.operator bool());

    while (!terminating_.load()
           && project_
           && project_->wait_ready())
    {
      emit plot_ready();
      if (!terminating_.load())
        QThread::sleep(wait_s_.load());
    }
//    DBG << "<ThreadPlot> loop ended";
  }

private:
  QMutex mutex_;
  DAQuiri::ProjectPtr project_;
  boost::atomic<bool> terminating_;
  boost::atomic<uint16_t> wait_s_;

  void terminate_helper()
  {
    terminating_.store(true);
    if (project_)
      project_->activate();
    wait();
    project_ = DAQuiri::ProjectPtr();
  }

};
