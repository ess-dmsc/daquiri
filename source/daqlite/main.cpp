
#include <QApplication>
#include <QPlot/QPlot.h>

#include <Configuration.h>
#include <MainWindow.h>
#include <Custom2DPlot.h>
#include <WorkerThread.h>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  Configuration Config(25, 25, "NMX_detector");

  MainWindow GUI(Config);

  app.exec();
  printf("app exited\n");
}
