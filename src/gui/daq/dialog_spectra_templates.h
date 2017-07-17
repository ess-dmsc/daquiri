#pragma once

#include <QDialog>
#include <QAbstractTableModel>
#include <QItemSelectionModel>
#include "special_delegate.h"
#include "qt_util.h"
#include "tree_settings.h"
#include "special_delegate.h"
#include "consumer_metadata.h"

namespace Ui {
class DialogSpectraTemplates;
class DialogSpectrumTemplate;
}


class TableSpectraTemplates : public QAbstractTableModel
{
  Q_OBJECT
private:
  Container<DAQuiri::ConsumerMetadata> &templates_;

public:
  TableSpectraTemplates(Container<DAQuiri::ConsumerMetadata>& templates, QObject *parent = 0);
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  Qt::ItemFlags flags(const QModelIndex & index) const;

  void update();
};


class DialogSpectraTemplates : public QDialog
{
  Q_OBJECT

public:
  explicit DialogSpectraTemplates(Container<DAQuiri::ConsumerMetadata> &newdb,
                                  std::vector<DAQuiri::Detector> current_dets,
                                  QString savedir, QWidget *parent = 0);
  ~DialogSpectraTemplates();

private:
  Ui::DialogSpectraTemplates *ui;

  Container<DAQuiri::ConsumerMetadata> &templates_;

  DAQuiriSpecialDelegate      special_delegate_;
  TableSpectraTemplates         table_model_;
  QItemSelectionModel selection_model_;

  QString root_dir_;
  std::vector<DAQuiri::Detector> current_dets_;

private slots:

  void on_pushImport_clicked();
  void on_pushExport_clicked();
  void on_pushNew_clicked();
  void on_pushEdit_clicked();
  void on_pushDelete_clicked();

  void on_pushSetDefault_clicked();
  void on_pushUseDefault_clicked();

  void selection_changed(QItemSelection,QItemSelection);
  void selection_double_clicked(QModelIndex);

  void toggle_push();
  void on_pushClear_clicked();
  void on_pushUp_clicked();
  void on_pushDown_clicked();
};
