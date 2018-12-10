#pragma once

#include <gui/widgets/SettingDelegate.h>
#include <gui/SettingsTreeModel.h>
#include <gui/ThreadRunner.h>
#include <core/detector.h>

#include <QWidget>
#include <QCloseEvent>
#include <QTableView>
#include <QTreeView>
#include <QMenu>

namespace Ui
{
class SettingsForm;
}

class SettingsForm : public QWidget
{
 Q_OBJECT

 public:
  explicit SettingsForm(ThreadRunner&,
                        QWidget* parent = 0);

  Setting get_tree()
  { return settings_tree_; }

  ~SettingsForm();

  void exit();

 public slots:
  void refresh();
  void update(const DAQuiri::Setting& tree,
              DAQuiri::ProducerStatus, DAQuiri::StreamManifest);

 signals:
  void toggleIO(bool);
  void requestList();

 protected:
  void closeEvent(QCloseEvent*);

 public slots:
  void updateDetDB();
  void on_pushChangeProfile_clicked();

 private slots:
  void begin_editing();
  void stop_editing(QWidget*, QAbstractItemDelegate::EndEditHint);
  void on_pushSettingsRefresh_clicked();

  void toggle_push(bool enable, DAQuiri::ProducerStatus status,
                   DAQuiri::StreamManifest manifest);

  void push_settings();

  void ask_binary_tree(DAQuiri::Setting, QModelIndex index);
  void ask_execute_tree(DAQuiri::Setting, QModelIndex index);
  void ask_file_tree(DAQuiri::Setting, QModelIndex index);
  void ask_dir_tree(DAQuiri::Setting, QModelIndex index);
  void ask_gradient_tree(QString gname, QModelIndex index);

  void on_checkShowRO_clicked();
  void on_bootButton_clicked();

  void on_spinRefreshFrequency_valueChanged(int arg1);

  void profile_chosen(QString name, bool boot);
  void init_profile();

  void on_pushExpandAll_toggled(bool checked);

  void on_pushAddProducer_clicked();
  void on_pushRemoveProducer_clicked();

  void on_pushRequestList_clicked();

 private:
  Ui::SettingsForm* ui;

  DAQuiri::ProducerStatus current_status_;
  ThreadRunner& runner_thread_;
  bool editing_{false};
  bool exiting_{false};

  Setting settings_tree_;
  SettingsTreeModel tree_settings_model_;
  SettingDelegate tree_delegate_;

  void loadSettings();
  void saveSettings();
};
