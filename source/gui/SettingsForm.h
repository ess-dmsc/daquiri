#pragma once

#include <QWidget>
#include <QCloseEvent>
#include <QTableView>
#include <QTreeView>
#include <QMenu>
#include "detector.h"
#include "SettingDelegate.h"
#include "SettingsTreeModel.h"

#include "ThreadRunner.h"

namespace Ui {
class SettingsForm;
}

class SettingsForm : public QWidget
{
    Q_OBJECT

  public:
    explicit SettingsForm(ThreadRunner&,
                          Container<Detector>&,
                          QWidget *parent = 0);
    Setting get_tree() {return settings_tree_;}
    ~SettingsForm();

    void exit();

  public slots:
    void refresh();
    void update(const DAQuiri::Setting &tree,
                const std::vector<DAQuiri::Detector> &channelsupdate,
                DAQuiri::ProducerStatus);

  signals:
    void toggleIO(bool);
    void requestList();

  protected:
    void closeEvent(QCloseEvent*);

  public slots:
    void updateDetDB();

  private slots:
    void begin_editing();
    void stop_editing(QWidget*,QAbstractItemDelegate::EndEditHint);
    void on_pushSettingsRefresh_clicked();

    void toggle_push(bool enable, DAQuiri::ProducerStatus status);

    void push_settings();
    void chose_detector(int chan, std::string name);

    void ask_binary_tree(DAQuiri::Setting, QModelIndex index);
    void ask_execute_tree(DAQuiri::Setting, QModelIndex index);

    void on_checkShowRO_clicked();
    void on_bootButton_clicked();

    void on_spinRefreshFrequency_valueChanged(int arg1);

    void on_pushChangeProfile_clicked();

    void profile_chosen(QString name, bool boot);
    void init_profile();

    void on_pushExpandAll_clicked();

    void on_pushAddProducer_clicked();
    void on_pushRemoveProducer_clicked();

    void on_pushRequestList_clicked();

  private:
    Ui::SettingsForm *ui;

    DAQuiri::ProducerStatus current_status_;
    Container<Detector>     &detectors_;
    ThreadRunner            &runner_thread_;
    bool editing_ {false};
    bool exiting_ {false};

    Setting           settings_tree_;
    SettingsTreeModel tree_settings_model_;
    SettingDelegate   tree_delegate_;

    void loadSettings();
    void saveSettings();
};
