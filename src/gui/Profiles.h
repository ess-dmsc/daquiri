#pragma once

#include <QDialog>
#include <QItemSelectionModel>
#include "setting.h"

namespace Ui {
class WidgetProfiles;
}

class WidgetProfiles : public QDialog
{
    Q_OBJECT

  public:
    explicit WidgetProfiles(QWidget *parent = 0);
    ~WidgetProfiles();

  signals:
    void profileChosen();

  private:
    Ui::WidgetProfiles *ui;

    std::vector<std::string> profile_dirs_;

    void update_profiles();
    void apply_selection(size_t i, bool boot);

    static QString settings_dir();
    static QString profiles_dir();
    static QString current_profile_dir();

  private slots:
    void selection_changed(QItemSelection,QItemSelection);
    void selection_double_clicked(QModelIndex);
    void on_pushApply_clicked();
    void on_pushApplyBoot_clicked();
    void on_OutputDirFind_clicked();
    void on_pushCreate_clicked();
    void on_pushDelete_clicked();
};
