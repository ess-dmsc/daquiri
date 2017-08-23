#include "ConsumerDialog.h"
#include "ui_ConsumerDialog.h"
#include "qt_util.h"
#include "custom_logger.h"
#include <QInputDialog>
#include <QMessageBox>
#include "consumer_factory.h"
//#include "dialog_detector.h"

using namespace DAQuiri;

ConsumerDialog::ConsumerDialog(ConsumerMetadata sink_metadata,
                               std::vector<Detector> current_detectors,
                               Container<Detector>& detDB,
                               bool has_sink_parent,
                               bool allow_edit_type,
                               QWidget *parent) :
  QDialog(parent),
  sink_metadata_(sink_metadata),
//  det_selection_model_(&det_table_model_),
  current_detectors_(current_detectors),
  changed_(false),
  has_sink_parent_(has_sink_parent),
  attr_model_(this),
  detectors_(detDB),
  ui(new Ui::ConsumerDialog)
{
  ui->setupUi(this);
  ui->labelWarning->setVisible(false);
  for (auto &q : ConsumerFactory::singleton().types())
    ui->comboType->addItem(QString::fromStdString(q));

  ui->treeAttribs->setEditTriggers(QAbstractItemView::AllEditTriggers);

  ui->pushLock->setVisible(has_sink_parent);
  ui->comboType->setEnabled(allow_edit_type);
  ui->widgetDetectors->setVisible(!allow_edit_type);

//  connect(&det_selection_model_, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
//          this, SLOT(det_selection_changed(QItemSelection,QItemSelection)));

  connect(&attr_model_, SIGNAL(tree_changed()), this, SLOT(push_settings()));

  ui->treeAttribs->setModel(&attr_model_);
  ui->treeAttribs->setItemDelegate(&attr_delegate_);
  ui->treeAttribs->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

//  det_table_model_.setDB(spectrum_detectors_);

//  ui->tableDetectors->setModel(&det_table_model_);
//  ui->tableDetectors->setSelectionModel(&det_selection_model_);
//  ui->tableDetectors->horizontalHeader()->setStretchLastSection(true);
//  ui->tableDetectors->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//  ui->tableDetectors->setSelectionBehavior(QAbstractItemView::SelectRows);
//  ui->tableDetectors->setSelectionMode(QAbstractItemView::SingleSelection);
//  ui->tableDetectors->show();

  attr_model_.set_show_address_(false);

  if (sink_metadata_ == ConsumerMetadata())
  {
    ui->spinDets->setValue(current_detectors_.size());
    on_comboType_activated(ui->comboType->currentText());
    Setting col = sink_metadata_.get_attribute("appearance");
    col.set_text(generateColor().name(QColor::HexArgb).toStdString());
    sink_metadata_.set_attribute(col);
  }

  updateData();
}

ConsumerDialog::~ConsumerDialog()
{
  delete ui;
}

void ConsumerDialog::det_selection_changed(QItemSelection, QItemSelection)
{
  toggle_push();
}

void ConsumerDialog::updateData()
{
  ui->comboType->setCurrentText(QString::fromStdString(sink_metadata_.type()));

  spectrum_detectors_.clear();
  for (auto &q: sink_metadata_.detectors)
    spectrum_detectors_.add_a(q);
//  det_table_model_.update();

  Setting pat = sink_metadata_.get_attribute("pattern_add");
  ui->spinDets->setValue(pat.pattern().gates().size());

  QString descr;
  auto sptr = ConsumerFactory::singleton().create_from_prototype(sink_metadata_);
  if (sptr)
  {
    auto md = sptr->metadata();
    descr = QString::fromStdString(md.type_description())
//        + "   dimensions=" + QString::number(md.dimensions())
        + "\n";
  }

  ui->labelDescription->setText(descr);

  attr_model_.update(sink_metadata_.attributes());
  open_close_locks();
}

void ConsumerDialog::open_close_locks() {
  bool lockit = !ui->pushLock->isChecked();
  ui->labelWarning->setVisible(lockit);
  ui->spinDets->setEnabled(lockit || !has_sink_parent_);

  ui->treeAttribs->clearSelection();
//  ui->tableDetectors->clearSelection();
  if (!lockit)
  {
    attr_model_.set_edit_read_only(false);
//    ui->tableDetectors->setSelectionMode(QAbstractItemView::NoSelection);
  }
  else
  {
    attr_model_.set_edit_read_only(true);
//    ui->tableDetectors->setSelectionMode(QAbstractItemView::SingleSelection);
    changed_ = true;
  }
  toggle_push();
}

void ConsumerDialog::push_settings()
{
  sink_metadata_.overwrite_all_attributes(attr_model_.get_tree());
  changed_ = true;
}

void ConsumerDialog::toggle_push()
{
  ui->pushDetEdit->setEnabled(false);
  ui->pushDetRename->setEnabled(false);
  ui->pushDetFromDB->setEnabled(false);
  ui->pushDetToDB->setEnabled(false);

//  bool unlocked = !ui->pushLock->isChecked();

//  QModelIndexList ixl = ui->tableDetectors->selectionModel()->selectedRows();
//  if (ixl.empty())
//    return;
//  int i = ixl.front().row();

//  if (i < static_cast<int>(sink_metadata_.detectors.size()))
//  {
//    ui->pushDetEdit->setEnabled(unlocked);
//    ui->pushDetRename->setEnabled(unlocked);
//    ui->pushDetToDB->setEnabled(unlocked);
//    Detector det = sink_metadata_.detectors[i];
//    if (unlocked && detectors_.has_a(det))
//      ui->pushDetFromDB->setEnabled(true);
//  }
}

void ConsumerDialog::on_pushLock_clicked()
{
  open_close_locks();
}

void ConsumerDialog::on_buttonBox_accepted()
{
  if (!changed_)
    reject();
  else
  {
    ConsumerPtr newsink = ConsumerFactory::singleton().create_from_prototype(sink_metadata_);

    if (!newsink)
    {
      QMessageBox msgBox;
      msgBox.setText("Attributes invalid for this type. Check requirements.");
      msgBox.exec();
      return;
    }
    else
      accept();
  }
}

void ConsumerDialog::on_buttonBox_rejected()
{
  reject();
}

void ConsumerDialog::on_pushDetEdit_clicked()
{
//  QModelIndexList ixl = ui->tableDetectors->selectionModel()->selectedRows();
//  if (ixl.empty())
//    return;
//  int i = ixl.front().row();

//  DialogDetector* newDet = new DialogDetector(spectrum_detectors_.get(i), false, this);
//  connect(newDet, SIGNAL(newDetReady(Detector)), this, SLOT(changeDet(Detector)));
//  newDet->exec();
}

void ConsumerDialog::changeDet(Detector newDetector)
{
//  QModelIndexList ixl = ui->tableDetectors->selectionModel()->selectedRows();
//  if (ixl.empty())
//    return;
//  int i = ixl.front().row();

//  if (i < static_cast<int>(sink_metadata_.detectors.size()))
//  {
//    sink_metadata_.detectors[i] = newDetector;
//    changed_ = true;

//    spectrum_detectors_.clear();
//    for (auto &q: sink_metadata_.detectors)
//      spectrum_detectors_.add_a(q);
////    det_table_model_.update();
//    open_close_locks();
//  }
}

void ConsumerDialog::on_pushDetRename_clicked()
{
//  QModelIndexList ixl = ui->tableDetectors->selectionModel()->selectedRows();
//  if (ixl.empty())
//    return;
//  int i = ixl.front().row();

//  bool ok;
//  QString text = QInputDialog::getText(this, "Rename Detector",
//                                       "Detector name:", QLineEdit::Normal,
//                                       QString::fromStdString(spectrum_detectors_.get(i).name()),
//                                       &ok);
//  if (ok && !text.isEmpty())
//  {
//    if (i < static_cast<int>(sink_metadata_.detectors.size()))
//    {
//      sink_metadata_.detectors[i].set_name(text.toStdString());
//      changed_ = true;

//      spectrum_detectors_.clear();
//      for (auto &q: sink_metadata_.detectors)
//        spectrum_detectors_.add_a(q);
////      det_table_model_.update();
//      open_close_locks();
//    }
//  }
}

void ConsumerDialog::on_pushDetFromDB_clicked()
{
//  QModelIndexList ixl = ui->tableDetectors->selectionModel()->selectedRows();
//  if (ixl.empty())
//    return;
//  int i = ixl.front().row();

//  if (i < static_cast<int>(sink_metadata_.detectors.size()))
//  {
//    Detector newdet = detectors_.get(sink_metadata_.detectors[i]);
//    sink_metadata_.detectors[i] = newdet;
//    changed_ = true;

//    spectrum_detectors_.clear();
//    for (auto &q: sink_metadata_.detectors)
//      spectrum_detectors_.add_a(q);
////    det_table_model_.update();
//    open_close_locks();
//  }
}

void ConsumerDialog::on_pushDetToDB_clicked()
{
//  QModelIndexList ixl = ui->tableDetectors->selectionModel()->selectedRows();
//  if (ixl.empty())
//    return;
//  int i = ixl.front().row();

//  if (i < static_cast<int>(sink_metadata_.detectors.size()))
//  {
//    Detector newdet = sink_metadata_.detectors[i];

//    if (!detectors_.has_a(newdet)) {
//      bool ok;
//      QString text = QInputDialog::getText(this, "New Detector",
//                                           "Detector name:", QLineEdit::Normal,
//                                           QString::fromStdString(newdet.name()),
//                                           &ok);

//      if (!ok)
//        return;

//      if (!text.isEmpty()) {
//        if (text.toStdString() != newdet.name())
//          newdet.set_name(text.toStdString());

//        if (detectors_.has_a(newdet)) {
//          QMessageBox::StandardButton reply = QMessageBox::question(this, "Replace existing?",
//                                        "Detector " + text + " already exists. Replace?",
//                                         QMessageBox::Yes|QMessageBox::No);
//          if (reply == QMessageBox::No)
//            return;
//          else {
//            detectors_.replace(newdet);
//            if (sink_metadata_.detectors[i].name() != newdet.name())
//            {
//              sink_metadata_.detectors[i] = newdet;
//              changed_ = true;
////              updateData();
//            }
//          }
//        } else
//          detectors_.replace(newdet);
//      }
//    } else {
//      QMessageBox::StandardButton reply = QMessageBox::question(this, "Replace existing?",
//                                    "Detector " + QString::fromStdString(newdet.name()) + " already exists. Replace?",
//                                     QMessageBox::Yes|QMessageBox::No);
//      if (reply == QMessageBox::No)
//        return;
//      else
//        detectors_.replace(newdet);
//    }
//  }
}

void ConsumerDialog::on_spinDets_valueChanged(int arg1)
{
  Setting pat = sink_metadata_.get_attribute("pattern_add");

  sink_metadata_.set_det_limit(arg1);
  if (arg1 != static_cast<int>(pat.pattern().gates().size()))
    changed_ = true;

  attr_model_.update(sink_metadata_.attributes());
  open_close_locks();
}


void ConsumerDialog::on_comboType_activated(const QString &arg1)
{
  ConsumerMetadata md = ConsumerFactory::singleton().create_prototype(arg1.toStdString());
  if (md != ConsumerMetadata())
  {
//    md.set_attributes(sink_metadata_.attributes());

    sink_metadata_ = md;

    on_spinDets_valueChanged(ui->spinDets->value());
    updateData();
  }
  else
    WARN << "Problem with spectrum type. Factory cannot make template for " << arg1.toStdString();
}
