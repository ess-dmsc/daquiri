#pragma once

#include <QWidget>
#include <QMenu>
#include "project.h"
#include "ThreadRunner.h"
#include "ThreadPlotSignal.h"

namespace Ui {
class FormMcaDaq;
}

class FormMcaDaq : public QWidget
{
  Q_OBJECT

public:
  explicit FormMcaDaq(ThreadRunner&,
                      Container<DAQuiri::Detector>&,
                      std::vector<DAQuiri::Detector>&,
                      DAQuiri::ProjectPtr,
                      QWidget *parent = 0);

  void replot();
  ~FormMcaDaq();

signals:
  void toggleIO(bool);
  void statusText(QString);
  void requestClose(QWidget*);

protected:
  void closeEvent(QCloseEvent*);

public slots:
  void toggle_push(bool, DAQuiri::ProducerStatus);

private slots:
  void on_pushMcaStart_clicked();
  void on_pushMcaStop_clicked();
  void run_completed();
  void on_pushEditSpectra_clicked();
  void update_plots();
  void clearGraphs();

  void start_DAQ();
  void newProject();
  void updateSpectraUI();

  void on_pushEnable2d_clicked();
  void on_pushForceRefresh_clicked();

//  void on_pushDetails_clicked();

  void on_toggleIndefiniteRun_clicked();

  void projectSave();
  void projectSaveAs();
  void projectOpen();

private:
  Ui::FormMcaDaq *ui;
  ThreadRunner               &runner_thread_;
  Container<DAQuiri::Detector> &detectors_;
  std::vector<DAQuiri::Detector> &current_dets_;

  QString data_directory_;    //data directory
  QString profile_directory_;

  Container<DAQuiri::ConsumerMetadata>  spectra_templates_;
  DAQuiri::ProjectPtr                   project_;

  ThreadPlotSignal                plot_thread_;
  boost::atomic<bool>             interruptor_;

  bool my_run_;

  QMenu  menuLoad;
  QMenu  menuSave;

  void loadSettings();
  void saveSettings();
};
