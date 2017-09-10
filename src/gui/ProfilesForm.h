#pragma once

#include <QDialog>
#include <QItemSelection>
#include <QDir>
#include "setting.h"

namespace Ui {
class ProfilesForm;
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


class ProfilesForm : public QDialog
{
    Q_OBJECT

  public:
    explicit ProfilesForm(QWidget *parent = 0);
    ~ProfilesForm();

  signals:
    void profileChosen();

  private:
    Ui::ProfilesForm *ui;

    struct ProfileEntry
    {
        ProfileEntry() {}
        ProfileEntry(QString i, QString d)
          : id(i), description(d) {}

        QString id;
        QString description;
    };

    QVector<ProfileEntry> profiles_;

    void update_profiles();
    void apply_selection(size_t i, bool boot);
    void create_profile();

  private slots:
    void selection_double_clicked(QModelIndex);
    void on_pushSelectRoot_clicked();
    void select_no_boot();
    void select_and_boot();
    void remove_profile();
};
