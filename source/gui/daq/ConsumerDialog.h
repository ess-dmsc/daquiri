#pragma once

#include <QItemSelection>
#include <QDialog>
#include <widgets/SettingDelegate.h>
#include "SettingsTreeModel.h"
//#include "widget_detectors.h"
#include <core/consumer.h>
#include <core/spill.h>

namespace Ui
{
class ConsumerDialog;
}

class ConsumerDialog : public QDialog
{
 Q_OBJECT

 public:
  explicit ConsumerDialog(DAQuiri::ConsumerPtr consumer,
                          std::vector<DAQuiri::Detector> current_detectors,
                          Container<DAQuiri::Detector>& detDB,
                          DAQuiri::StreamManifest stream_manifest,
                          bool allow_edit_type,
                          QWidget* parent = 0);
  ~ConsumerDialog();

  DAQuiri::ConsumerPtr product();

 private slots:
  void on_buttonBox_accepted();
  void on_buttonBox_rejected();

  void on_pushLock_clicked();

  void on_pushDetEdit_clicked();
  void on_pushDetRename_clicked();
  void on_pushDetFromDB_clicked();
  void on_pushDetToDB_clicked();
  void on_spinDets_valueChanged(int arg1);

  void on_pushExpandAll_toggled(bool checked);

  void push_settings();
  void changeDet(DAQuiri::Detector);
  void det_selection_changed(QItemSelection, QItemSelection);

  void on_comboType_activated(const QString& arg1);

  void ask_gradient(QString gname, QModelIndex index);
  void ask_file(DAQuiri::Setting, QModelIndex index);
  void ask_dir(DAQuiri::Setting, QModelIndex index);

 private:
  Ui::ConsumerDialog* ui;
  DAQuiri::ConsumerPtr consumer_;

  SettingsTreeModel attr_model_;
  SettingDelegate attr_delegate_;

  Container<DAQuiri::Detector>& detectors_;
  std::vector<DAQuiri::Detector> current_detectors_;
  StreamManifest stream_manifest_;

  Container<DAQuiri::Detector> spectrum_detectors_;
//    TableDetectors det_table_model_;
//    QItemSelectionModel det_selection_model_;

  bool changed_;
  bool has_consumer_parent_;

  void updateData();
  void open_close_locks();
  void toggle_push();

  static void initialize_gui_specific(DAQuiri::ConsumerMetadata& md);

  void enforce_streams(DAQuiri::Setting& tree, DAQuiri::StreamManifest stream_manifest);

  void enforce_everything();

  void loadSettings();
  void saveSettings();

};
