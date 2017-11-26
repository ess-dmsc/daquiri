#pragma once

#include <QWidget>
#include "spill.h"
#include "ThreadRunner.h"
#include "SettingDelegate.h"
//#include "widget_detectors.h"
#include "SettingsTreeModel.h"

#include <QItemSelectionModel>
#include <QAbstractTableModel>


namespace Ui {
class ListModeForm;
}

class ListModeForm : public QWidget
{
  Q_OBJECT

public:
  explicit ListModeForm(ThreadRunner&, QWidget *parent = 0);
  ~ListModeForm();

signals:
  void toggleIO(bool);

private slots:
  void spillSelectionChanged(int);
  void event_selection_changed(QItemSelection,QItemSelection);
  void stats_selection_changed(QItemSelection,QItemSelection);
  void toggle_push(bool online, DAQuiri::ProducerStatus);

  void on_pushListStart_clicked();
  void on_pushListStop_clicked();
  void list_completed(DAQuiri::ListData);

protected:
  void closeEvent(QCloseEvent*);

private:
  Ui::ListModeForm     *ui;
  ThreadRunner        &runner_thread_;
  Interruptor interruptor_;
  bool my_run_;

  DAQuiri::ListData     list_data_;

  std::vector<DAQuiri::Event>      hits_;
  std::vector<DAQuiri::Detector> dets_;
  std::map<int16_t, DAQuiri::EventModel> hitmodels_;
  std::map<int16_t, DAQuiri::Status> stats_;
  Container<DAQuiri::Detector> spill_detectors_;

//  TableDetectors det_table_model_;
  SettingsTreeModel               attr_model_;
  SettingDelegate     attr_delegate_;


  void displayHit(int idx);
  void displayStats(int idx);

  void loadSettings();
  void saveSettings();
};
