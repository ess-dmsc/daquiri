// Copyright (C) 2020 - 2022 European Spallation Source, ERIC. See LICENSE file
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

  while (true) {
    auto Msg = Consumer->consume();

    Consumer->handleMessage(Msg);

    t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<int64_t, std::nano> elapsed = t2 - t1;

    /// once every ~ 1 second, copy the histograms and tell main thread
    /// that plots can be updated.
    if (elapsed.count() >= 1000000000LL) {

      mutex.lock();
      Consumer->mHistogramPlot = Consumer->mHistogram;
      Consumer->mHistogramTofPlot = Consumer->mHistogramTof;
      Consumer->mPixelIDsPlot = Consumer->mPixelIDs;
      Consumer->mTOFsPlot = Consumer->mTOFs;
      mutex.unlock();

      int ElapsedCountMS = elapsed.count()/1000000;
      emit resultReady(ElapsedCountMS);

      std::fill(Consumer->mHistogram.begin(), Consumer->mHistogram.end(), 0);
      std::fill(Consumer->mHistogramTof.begin(), Consumer->mHistogramTof.end(), 0);
      Consumer->mPixelIDs.clear();
      Consumer->mTOFs.clear();
      t1 = std::chrono::high_resolution_clock::now();
    }
    delete Msg;
  }
}
