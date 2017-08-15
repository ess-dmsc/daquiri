#pragma once

#include <QDialog>
#include <QAbstractTableModel>
#include <QItemSelectionModel>
#include "setting.h"

namespace Ui {
class WidgetProfiles;
}

class TableProfiles : public QAbstractTableModel
{
    Q_OBJECT
  private:
    std::vector<DAQuiri::Setting> &profiles_;
  public:
    TableProfiles(std::vector<DAQuiri::Setting>& pf, QObject *parent = 0): QAbstractTableModel(parent), profiles_(pf) {}
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex & index) const;

    void update();
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

    std::vector<std::string> profile_dirs_;
    std::vector<DAQuiri::Setting> profiles_;

    TableProfiles table_model_;
    QItemSelectionModel selection_model_;

    void update_profiles();

  private slots:
    void selection_changed(QItemSelection,QItemSelection);
    void selection_double_clicked(QModelIndex);
    void toggle_push();
    void on_pushApply_clicked();
    void on_pushApplyBoot_clicked();
    void on_OutputDirFind_clicked();
};
