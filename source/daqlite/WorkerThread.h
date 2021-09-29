/* Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file      */
//===----------------------------------------------------------------------===//
///
/// \file WorkerThread.h
///
/// \brief main consumer loop for Daquiri Light (daqlite)
/// The worker thread continuously calls ESSConsumer::consume() and
/// ESSConsumer::handleMessage() to histogram the pixelids. Once every second
/// the histogram is copied and the plotting thread (qt main thread?) is
/// notified.
//===----------------------------------------------------------------------===//

#pragma once

#include <ESSConsumer.h>
#include <QThread>
#include <QMutex>
#include <iostream>

class WorkerThread : public QThread {
  Q_OBJECT

public:
  WorkerThread(Configuration &Config) : mConfig(Config) {
    Consumer = new ESSConsumer(Config);
  };

  ~WorkerThread(){};

  /// \brief thread main loop
  void run() override;

  /// \brief Getter for the consumer
  ESSConsumer * consumer() { return Consumer; }


  QMutex mutex;

signals:
  void resultReady(uint64_t &val);

private:
  /// \brief configuration obtained from main()
  Configuration &mConfig;

  /// \brief Kafka consumer
  ESSConsumer *Consumer;


};
