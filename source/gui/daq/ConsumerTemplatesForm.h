#pragma once

#include <QDialog>
#include <QAbstractTableModel>
#include <QItemSelectionModel>
#include <gui/widgets/SettingDelegate.h>
#include <gui/widgets/qt_util.h>
#include <gui/SettingsTreeModel.h>
#include <core/project.h>

namespace Ui {
class ConsumerTemplatesForm;

class ConsumerDialogTemplate;
}

class ConsumerTemplatesTableModel : public QAbstractTableModel
{
  Q_OBJECT
  private:
    Container<ConsumerPtr> consumers_;

  public:
    ConsumerTemplatesTableModel(QObject* parent = 0);
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;

    void update(DAQuiri::ProjectPtr& project);
};

class ConsumerTemplatesForm : public QDialog
{
  Q_OBJECT

  public:
    explicit ConsumerTemplatesForm(DAQuiri::ProjectPtr& project,
                                   std::vector<DAQuiri::Detector> current_dets,
                                   DAQuiri::StreamManifest stream_manifest,
                                   QString data_dir, QString profile_dir, QWidget* parent = 0);
    ~ConsumerTemplatesForm();

  protected:
    void closeEvent(QCloseEvent*) Q_DECL_OVERRIDE;

  private:
    Ui::ConsumerTemplatesForm* ui;

    DAQuiri::ProjectPtr project_;

    SettingDelegate special_delegate_;
    ConsumerTemplatesTableModel table_model_;
    QItemSelectionModel selection_model_;

    QString data_dir_;
    QString profile_dir_;
    std::vector<DAQuiri::Detector> current_dets_;
    DAQuiri::StreamManifest stream_manifest_;

  private slots:
    void on_pushImport_clicked();
    void on_pushNew_clicked();
    void on_pushEdit_clicked();
    void on_pushDelete_clicked();

    void on_pushSetDefault_clicked();
    void on_pushUseDefault_clicked();

    void selection_changed(QItemSelection, QItemSelection);
    void selection_double_clicked(QModelIndex);

    void toggle_push();
    void on_pushClear_clicked();
    void on_pushUp_clicked();
    void on_pushDown_clicked();
    void on_pushClone_clicked();

    void loadSettings();
    void saveSettings();
};
