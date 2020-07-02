
#include <WorkerThread.h>

void WorkerThread::run() {
  int i = 0;
  while (1) {
    auto Msg = Consumer->consume();
    bool HasData = Consumer->handleMessage(Msg, nullptr);
    delete Msg;
    if (HasData) {
      emit resultReady(i);
      i += 1;
    }
    // QThread::msleep(1000);
  }
}
