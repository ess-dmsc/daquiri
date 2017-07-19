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
class FormListDaq;
}

class FormListDaq : public QWidget
{
  Q_OBJECT

public:
  explicit FormListDaq(ThreadRunner&, QWidget *parent = 0);
  ~FormListDaq();

signals:
  void toggleIO(bool);
  void statusText(QString);

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
  Ui::FormListDaq     *ui;
  ThreadRunner        &runner_thread_;
  bool my_run_;
  Interruptor interruptor_;


  DAQuiri::ListData     list_data_;


  std::vector<DAQuiri::Event>      hits_;
  std::vector<DAQuiri::Detector> dets_;
  std::map<int16_t, DAQuiri::EventModel> hitmodels_;
  std::map<int16_t, DAQuiri::Status> stats_;
  Container<DAQuiri::Detector> spill_detectors_;

//  TableDetectors det_table_model_;
  TreeSettings               attr_model_;
  DAQuiriSpecialDelegate     attr_delegate_;


  void displayHit(int idx);
  void displayStats(int idx);

  void loadSettings();
  void saveSettings();
};
