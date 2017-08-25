#pragma once

#include <QDialog>
#include <QItemSelection>
#include <QDir>
#include "setting.h"

namespace Ui {
class WidgetProfiles;
}

class ProfileDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit ProfileDialog(QString description, QWidget *parent = 0);

  signals:
    void load();
    void boot();
    void remove();

  private slots:
    void clickedLoad();
    void clickedBoot();
    void clickedRemove();
    void clickedCancel();
};


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

    struct ProfileEntry
    {
        ProfileEntry() {}
        ProfileEntry(QDir p, QString d)
          : path(p), description(d) {}

        QDir path;
        QString description;
    };

    QVector<ProfileEntry> profiles_;

    void update_profiles();
    void apply_selection(size_t i, bool boot);

    static QString settings_dir();
    static QString profiles_dir();
    static QString current_profile_dir();

  private slots:
    void selection_double_clicked(QModelIndex);
    void on_pushApply_clicked();
    void on_pushApplyBoot_clicked();
    void on_OutputDirFind_clicked();
    void on_pushCreate_clicked();
    void on_pushDelete_clicked();
};
