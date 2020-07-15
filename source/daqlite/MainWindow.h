#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Configuration.h>
#include <Custom2DPlot.h>
#include <WorkerThread.h>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
  MainWindow(Configuration &Config, QWidget *parent = nullptr);
  ~MainWindow();


  // Create GUI layout
  void setupLayout();

  /// \brief spin up a thread for consuming topic
  void startKafkaConsumerThread();

public slots:
  void handleExitButton();
  void handleKafkaData(int EventRate);

private:
  Ui::MainWindow *ui;

  /// \brief
  Custom2DPlot *Plot2D;

  /// \brief configuration obtained from main()
  Configuration mConfig;

  /// \brief
  WorkerThread *KafkaConsumerThread;
};
#endif // MAINWINDOW_H
