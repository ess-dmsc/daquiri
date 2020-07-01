
#include <WorkerThread.h>

void WorkerThread::run() {
  int i = 0;
  while (1) {
    Consumer.consume();
    emit resultReady(i);
    i += 1;
    QThread::msleep(200);
  }
}
