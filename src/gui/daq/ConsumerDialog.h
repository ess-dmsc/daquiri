#pragma once

#include <QItemSelection>
#include <QDialog>
#include "SettingDelegate.h"
#include "SettingsTreeModel.h"
//#include "widget_detectors.h"
#include "consumer_metadata.h"

namespace Ui {
class DialogSpectrum;
}

class DialogSpectrum : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSpectrum(DAQuiri::ConsumerMetadata sink_metadata,
                            std::vector<DAQuiri::Detector> current_detectors,
                            Container<DAQuiri::Detector>& detDB,
                            bool has_sink_parent,
                            bool allow_edit_type,
                            QWidget *parent = 0);
    ~DialogSpectrum();

    DAQuiri::ConsumerMetadata product() { return sink_metadata_; }

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

    void on_pushLock_clicked();

    void on_pushDetEdit_clicked();
    void on_pushDetRename_clicked();
    void on_pushDetFromDB_clicked();
    void on_pushDetToDB_clicked();
    void on_spinDets_valueChanged(int arg1);

    void push_settings();
    void changeDet(DAQuiri::Detector);
    void det_selection_changed(QItemSelection,QItemSelection);

    void on_comboType_activated(const QString &arg1);

private:
    Ui::DialogSpectrum *ui;
    DAQuiri::ConsumerMetadata sink_metadata_;

    TreeSettings               attr_model_;
    DAQuiriSpecialDelegate         attr_delegate_;

    Container<DAQuiri::Detector> &detectors_;
    std::vector<DAQuiri::Detector> current_detectors_;

    Container<DAQuiri::Detector> spectrum_detectors_;
//    TableDetectors det_table_model_;
//    QItemSelectionModel det_selection_model_;

    bool changed_;
    bool has_sink_parent_;

    void updateData();
    void open_close_locks();
    void toggle_push();
};
