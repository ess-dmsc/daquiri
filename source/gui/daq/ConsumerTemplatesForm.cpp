#include "ConsumerTemplatesForm.h"
#include "ui_ConsumerTemplatesForm.h"
#include "ConsumerDialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include<QSettings>
#include <project.h>
#include "json_file.h"

#include "QFileExtensions.h"
#include "consumer_factory.h"

using namespace DAQuiri;

ConsumerTemplatesTableModel::ConsumerTemplatesTableModel(QObject *parent)
  : QAbstractTableModel(parent)
{
}

int ConsumerTemplatesTableModel::rowCount(const QModelIndex & /*parent*/) const
{
  return consumers_.size();
}

int ConsumerTemplatesTableModel::columnCount(const QModelIndex & /*parent*/) const
{
  return 7;
}

QVariant ConsumerTemplatesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole)
  {
    if (orientation == Qt::Horizontal) {
      switch (section)
      {
      case 0:
        return QString("name");
      case 1:
        return QString("visible");
      case 2:
        return QString("type");
      case 3:
        return QString("stream");
      case 4:
        return QString("appearance");
      case 5:
        return QString("scale");
      }
    } else if (orientation == Qt::Vertical) {
      return QString::number(section);
    }
  }
  return QVariant();
}

QVariant ConsumerTemplatesTableModel::data(const QModelIndex &index, int role) const
{
  int row = index.row();
  int col = index.column();

  if (role == Qt::DisplayRole)
  {
    switch (col)
    {
    case 0:
      return QVariant::fromValue(consumers_.get(row)->metadata().get_attribute("name"));
    case 1:
      return QVariant::fromValue(consumers_.get(row)->metadata().get_attribute("visible"));
    case 2:
      return QString::fromStdString(consumers_.get(row)->metadata().type());
    case 3:
      return QVariant::fromValue(consumers_.get(row)->metadata().get_attribute("stream_id"));
    case 4:
      return QVariant::fromValue(consumers_.get(row)->metadata().get_attribute("appearance"));
    case 5:
      return QVariant::fromValue(consumers_.get(row)->metadata().get_attribute("preferred_scale"));
    }
  }
  return QVariant();
}

void ConsumerTemplatesTableModel::update(DAQuiri::ProjectPtr &project)
{
  consumers_ = project->get_consumers();
  QModelIndex start_ix = createIndex( 0, 0 );
  QModelIndex end_ix = createIndex(consumers_.size(), columnCount());
  emit dataChanged( start_ix, end_ix );
  emit layoutChanged();
}

Qt::ItemFlags ConsumerTemplatesTableModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags myflags =
      QAbstractTableModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  return myflags;
}






ConsumerTemplatesForm::ConsumerTemplatesForm(DAQuiri::ProjectPtr &project,
                                               std::vector<Detector> current_dets, StreamManifest stream_manifest,
                                               QString savedir, QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::ConsumerTemplatesForm)
    , project_(project)
  , selection_model_(&table_model_)
  , root_dir_(savedir)
  , current_dets_(current_dets)
  , stream_manifest_(stream_manifest)
{
  ui->setupUi(this);

  ui->spectraSetupView->setModel(&table_model_);
  ui->spectraSetupView->setItemDelegate(&special_delegate_);
  ui->spectraSetupView->setSelectionModel(&selection_model_);
  ui->spectraSetupView->verticalHeader()->hide();
  ui->spectraSetupView->horizontalHeader()->setStretchLastSection(true);
  ui->spectraSetupView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->spectraSetupView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->spectraSetupView->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->spectraSetupView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  ui->spectraSetupView->show();

  connect(&selection_model_, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(selection_changed(QItemSelection,QItemSelection)));
  connect(ui->spectraSetupView, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(selection_double_clicked(QModelIndex)));

  table_model_.update(project_);
  ui->spectraSetupView->resizeColumnsToContents();
  ui->spectraSetupView->resizeRowsToContents();

  loadSettings();
}

ConsumerTemplatesForm::~ConsumerTemplatesForm()
{
  delete ui;
}

void ConsumerTemplatesForm::loadSettings()
{
  QSettings settings;
  settings.beginGroup("DAQ_behavior");
  ui->checkAutosaveTemplates->setChecked(settings.value("autosave_templates", true).toBool());
  ui->checkAutosaveDAQ->setChecked(settings.value("autosave_daq", true).toBool());
  ui->checkConfirmTemplates->setChecked(settings.value("confirm_templates", true).toBool());
  ui->checkAskSaveProject->setChecked(settings.value("ask_save_project", true).toBool());
  settings.endGroup();
}

void ConsumerTemplatesForm::saveSettings()
{
  QSettings settings;
  settings.beginGroup("DAQ_behavior");
  settings.setValue("autosave_templates", ui->checkAutosaveTemplates->isChecked());
  settings.setValue("autosave_daq", ui->checkAutosaveDAQ->isChecked());
  settings.setValue("confirm_templates", ui->checkConfirmTemplates->isChecked());
  settings.setValue("ask_save_project", ui->checkAskSaveProject->isChecked());
  settings.endGroup();
}

void ConsumerTemplatesForm::selection_changed(QItemSelection, QItemSelection) {
  toggle_push();
}

void ConsumerTemplatesForm::toggle_push()
{
  if (selection_model_.selectedIndexes().empty())
  {
    ui->pushEdit->setEnabled(false);
    ui->pushDelete->setEnabled(false);
    ui->pushUp->setEnabled(false);
    ui->pushDown->setEnabled(false);  
    ui->pushClone->setEnabled(false);
  } else {
    ui->pushDelete->setEnabled(true);
  }

  if (selection_model_.selectedRows().size() == 1)
  {
    ui->pushEdit->setEnabled(true);
    ui->pushClone->setEnabled(true);
    QModelIndexList ixl = selection_model_.selectedRows();
    if (ixl.front().row() > 0)
      ui->pushUp->setEnabled(true);
    else
      ui->pushUp->setEnabled(false);
    if ((ixl.front().row() + 1) < static_cast<int>(project_->get_consumers().size()))
      ui->pushDown->setEnabled(true);
    else
      ui->pushDown->setEnabled(false);
  }

  if (project_->empty())
    ui->pushExport->setEnabled(false);
  else
    ui->pushExport->setEnabled(true);

}

void ConsumerTemplatesForm::on_pushImport_clicked()
{
  //ask clear or append?
  QString fileName = QFileDialog::getOpenFileName(this, "Load template spectra",
                                                  root_dir_, "Template set (*.tem)");
  if (validateFile(this, fileName, false))
  {
    INFO << "Reading templates from file " << fileName.toStdString();
    for (auto p : from_json_file(fileName.toStdString()))
      project_->add_consumer(ConsumerMetadata(p));

    selection_model_.reset();
    table_model_.update(project_);
    toggle_push();

    ui->spectraSetupView->horizontalHeader()->setStretchLastSection(true);
    ui->spectraSetupView->resizeColumnsToContents();
  }
}

void ConsumerTemplatesForm::on_pushExport_clicked()
{
  QString fileName = CustomSaveFileDialog(this, "Save template spectra",
                                          root_dir_, "Template set (*.tem)");
  if (validateFile(this, fileName, true))
  {
    INFO << "Writing templates to file " << fileName.toStdString();
    to_json_file(project_->get_prototypes(), fileName.toStdString());
  }
}

void ConsumerTemplatesForm::on_pushNew_clicked()
{
  Container<Detector> fakeDetDB;
  ConsumerDialog* newDialog =
      new ConsumerDialog(nullptr, current_dets_, fakeDetDB,
                         stream_manifest_, false, true, this);
  if (newDialog->exec())
  {
    project_->add_consumer(newDialog->product());

    selection_model_.reset();
    table_model_.update(project_);
    toggle_push();
  }
}

void ConsumerTemplatesForm::on_pushEdit_clicked()
{
  QModelIndexList ixl = ui->spectraSetupView->selectionModel()->selectedRows();
  if (ixl.empty())
    return;
  int i = ixl.front().row();
  Container<Detector> fakeDetDB;
  ConsumerDialog* newDialog =
      new ConsumerDialog(project_->get_consumer(i),
                         current_dets_, fakeDetDB,
                         stream_manifest_, true, false, this);
  if (newDialog->exec())
  {
    //project_->replace(i, newDialog->product());
    table_model_.update(project_);
    toggle_push();
  }
}

void ConsumerTemplatesForm::on_pushClone_clicked()
{
  QModelIndexList ixl = ui->spectraSetupView->selectionModel()->selectedRows();
  if (ixl.empty())
    return;
  int i = ixl.front().row();

  project_->add_consumer(project_->get_consumer(i)->metadata().prototype());
  table_model_.update(project_);
  toggle_push();
}

void ConsumerTemplatesForm::selection_double_clicked(QModelIndex)
{
  on_pushEdit_clicked();
}

void ConsumerTemplatesForm::on_pushDelete_clicked()
{
  QModelIndexList ixl = ui->spectraSetupView->selectionModel()->selectedRows();
  if (ixl.empty())
    return;

  std::list<int> torem;

  // must remove them in reverse order
  for (auto &ix : ixl)
    torem.push_front(ix.row());

  for (auto &i : torem)
    project_->delete_consumer(i);

  selection_model_.reset();
  table_model_.update(project_);
  toggle_push();
}

void ConsumerTemplatesForm::on_pushSetDefault_clicked()
{
  int reply = QMessageBox::warning(this, "Set default?",
                                   "Set current templates as default?",
                                   QMessageBox::Yes|QMessageBox::Cancel);
  if (reply != QMessageBox::Yes)
  {
    return;
  }
  save_default();
}

void ConsumerTemplatesForm::on_pushUseDefault_clicked()
{
  int reply = QMessageBox::warning(this, "Reset to defaults?",
                                   "Reset to default templete configuration?",
                                   QMessageBox::Yes|QMessageBox::Cancel);
  if (reply != QMessageBox::Yes)
  {
    return;
  }
  project_->set_prototypes(from_json_file(root_dir_.toStdString() + "/default_consumers.tem"));

  selection_model_.reset();
  table_model_.update(project_);
  toggle_push();
}

void ConsumerTemplatesForm::save_default()
{
  to_json_file(project_->get_prototypes(), root_dir_.toStdString() + "/default_consumers.tem");
}

void ConsumerTemplatesForm::on_pushClear_clicked()
{
  project_->clear();
  selection_model_.reset();
  table_model_.update(project_);
  toggle_push();
}

void ConsumerTemplatesForm::on_pushUp_clicked()
{
  QModelIndexList ixl = ui->spectraSetupView->selectionModel()->selectedRows();
  if (ixl.empty())
    return;

  project_->up(ixl.front().row());
  selection_model_.setCurrentIndex(ixl.front().sibling(ixl.front().row()-1, ixl.front().column()),
                                   QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
  table_model_.update(project_);
  toggle_push();
}

void ConsumerTemplatesForm::on_pushDown_clicked()
{
  QModelIndexList ixl = ui->spectraSetupView->selectionModel()->selectedRows();
  if (ixl.empty())
    return;

  project_->down(ixl.front().row());

  selection_model_.setCurrentIndex(ixl.front().sibling(ixl.front().row()+1, ixl.front().column()),
                                   QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
  table_model_.update(project_);
  toggle_push();
}

void ConsumerTemplatesForm::on_buttonBox_accepted()
{
  if (ui->checkAutosaveTemplates->isChecked())
  {
    save_default();
  }
  saveSettings();
}
