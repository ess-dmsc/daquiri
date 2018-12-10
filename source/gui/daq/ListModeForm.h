#pragma once

#include <QWidget>
#include <core/spill.h>
#include <gui/ThreadRunner.h>
#include <widgets/SettingDelegate.h>
//#include "widget_detectors.h"
#include <gui/SettingsTreeModel.h>

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
    void toggle_push(bool online, DAQuiri::ProducerStatus,
                     DAQuiri::StreamManifest);
    void spill_selection_changed(QItemSelection, QItemSelection);
    void event_selection_changed(QItemSelection, QItemSelection);
    void trace_selection_changed(QItemSelection, QItemSelection);

    void on_pushListStart_clicked();
    void on_pushListStop_clicked();
    void list_completed(DAQuiri::ListData);

  protected:
    void closeEvent(QCloseEvent*);

  private:
    Ui::ListModeForm* ui;

    ThreadRunner& runner_thread_;
    Interruptor   interruptor_;
    bool          my_run_;

    DAQuiri::ListData list_data_;

    DAQuiri::EventModel         event_model_;
    std::vector<DAQuiri::Event> events_;

    SettingsTreeModel   attr_model_;
    SettingDelegate     attr_delegate_;

    ConsumerPtr trace_;

    void displayHit(int idx);

    void loadSettings();
    void saveSettings();
};
