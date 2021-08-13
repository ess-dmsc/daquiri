/* Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file      */
//===----------------------------------------------------------------------===//
///
/// \file WorkerThread.cpp
///
//===----------------------------------------------------------------------===//

#include <WorkerThread.h>
#include <chrono>

void WorkerThread::run() {

  auto t2 = std::chrono::high_resolution_clock::now();
  auto t1 = std::chrono::high_resolution_clock::now();
  while (1) {
    auto Msg = Consumer->consume();

    /// \todo return counts so we can calculate count rates
    Consumer->handleMessage(Msg);

    t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<int64_t,std::nano> elapsed = t2 - t1;
    if (elapsed.count() >= 1000000000ULL) {
      mutex.lock();
      Consumer->mHistogramPlot = Consumer->mHistogram;
      Consumer->mHistogramTofPlot = Consumer->mHistogramTof;
      mutex.unlock();

      uint64_t Rate = (uint64_t)((Consumer->PixelCount * 1000000000ULL)/elapsed.count());
      emit resultReady(Rate);

      Consumer->PixelCount = 0;
      std::fill(Consumer->mHistogram.begin(), Consumer->mHistogram.end(), 0);
      std::fill(Consumer->mHistogramTof.begin(), Consumer->mHistogramTof.end(), 0);
      t1 = std::chrono::high_resolution_clock::now();
    }
    delete Msg;
  }
}
