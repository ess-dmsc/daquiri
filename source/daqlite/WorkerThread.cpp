
#include <WorkerThread.h>
#include <chrono>

void WorkerThread::run() {
  int i = 0;

  auto t2 = std::chrono::high_resolution_clock::now();
  auto t1 = std::chrono::high_resolution_clock::now();
  while (1) {
    auto Msg = Consumer->consume();
    bool HasData = Consumer->handleMessage(Msg, nullptr);

    t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<int64_t,std::nano> elapsed = t2 - t1;
    if (elapsed.count() >= 1000000000) {
      Consumer->mHistogramPlot = Consumer->mHistogram;

      emit resultReady(i);
      i += 1;

      std::fill(Consumer->mHistogram.begin(), Consumer->mHistogram.end(), 0);
      t1 = std::chrono::high_resolution_clock::now();
    }
    delete Msg;
  }
}
