
#include <QApplication>
#include <QPlot/QPlot.h>

#include <Configuration.h>
#include <Custom2DPlot.h>
#include <MainWindow.h>
#include <WorkerThread.h>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  Configuration Config(25, 25, "NMX_detector", "172.17.5.38:9092");
  MainWindow DaquiriLite(Config);

  return app.exec();
}
