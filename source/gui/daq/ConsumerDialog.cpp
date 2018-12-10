#include "ConsumerDialog.h"
#include "ui_ConsumerDialog.h"
#include <core/util/custom_logger.h>
#include <QInputDialog>
#include <QMessageBox>
#include <core/consumer_factory.h>
//#include "dialog_detector.h"

#include <widgets/qt_util.h>
#include <widgets/QColorExtensions.h>

#include <QPlot/GradientSelector.h>

using namespace DAQuiri;

ConsumerDialog::ConsumerDialog(ConsumerPtr consumer,
                               std::vector<Detector> current_detectors,
                               Container<Detector>& detDB,
                               StreamManifest stream_manifest,
                               bool allow_edit_type,
                               QWidget* parent)
    : QDialog(parent)
      , ui(new Ui::ConsumerDialog)
      , consumer_(consumer)
//,  det_selection_model_(&det_table_model_)
      , attr_model_(this)
      , detectors_(detDB)
      , current_detectors_(current_detectors)
      , stream_manifest_(stream_manifest)
      , changed_(false)
{
  ui->setupUi(this);
  loadSettings();

  auto consumer_types = ConsumerFactory::singleton().types();
  for (auto& q : consumer_types)
    ui->comboType->addItem(QS(q));

  std::string default_type = "";
  if (consumer_types.size())
    default_type = consumer_types[0];

  if (!consumer_)
    consumer_ = ConsumerFactory::singleton().create_type(default_type);

  ui->labelWarning->setVisible(false);
  ui->treeAttribs->setEditTriggers(QAbstractItemView::AllEditTriggers);

  ui->comboType->setEnabled(allow_edit_type);
  ui->widgetDetectors->setVisible(!allow_edit_type);

//  connect(&det_selection_model_, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
//          this, SLOT(det_selection_changed(QItemSelection,QItemSelection)));

  connect(&attr_model_, SIGNAL(tree_changed()), this, SLOT(push_settings()));

  attr_delegate_.set_manifest(stream_manifest);
  ui->treeAttribs->setModel(&attr_model_);
  ui->treeAttribs->setItemDelegate(&attr_delegate_);
  ui->treeAttribs->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

  connect(&attr_delegate_, SIGNAL(ask_gradient(QString, QModelIndex)),
          this, SLOT(ask_gradient(QString, QModelIndex)));
  connect(&attr_delegate_, SIGNAL(ask_file(DAQuiri::Setting, QModelIndex)),
          this, SLOT(ask_file(DAQuiri::Setting, QModelIndex)));
  connect(&attr_delegate_, SIGNAL(ask_dir(DAQuiri::Setting, QModelIndex)),
          this, SLOT(ask_dir(DAQuiri::Setting, QModelIndex)));

//  det_table_model_.setDB(spectrum_detectors_);

//  ui->tableDetectors->setModel(&det_table_model_);
//  ui->tableDetectors->setSelectionModel(&det_selection_model_);
//  ui->tableDetectors->horizontalHeader()->setStretchLastSection(true);
//  ui->tableDetectors->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//  ui->tableDetectors->setSelectionBehavior(QAbstractItemView::SelectRows);
//  ui->tableDetectors->setSelectionMode(QAbstractItemView::SingleSelection);
//  ui->tableDetectors->show();

  attr_model_.set_show_address(false);

  //this could be done better!!
  bool advanced = !consumer_->data()->empty();
  attr_model_.set_show_read_only(advanced);
  ui->pushLock->setVisible(advanced);

  if (!consumer_)
  {
    ui->spinDets->setValue(current_detectors_.size());
    on_comboType_activated(ui->comboType->currentText());
//    initialize_gui_specific(consumer_metadata_);
  }

  updateData();
  on_pushExpandAll_toggled(ui->pushExpandAll->isChecked());
}

ConsumerDialog::~ConsumerDialog()
{
  delete ui;
}

DAQuiri::ConsumerPtr ConsumerDialog::product()
{
  return consumer_;
}

void ConsumerDialog::det_selection_changed(QItemSelection, QItemSelection)
{
  toggle_push();
}

void ConsumerDialog::open_close_locks()
{
  bool lockit = !ui->pushLock->isChecked();
  ui->labelWarning->setVisible(lockit);
  ui->spinDets->setEnabled(lockit);

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

void ConsumerDialog::updateData()
{
  auto metadata = consumer_->metadata();

  ui->comboType->setCurrentText(QS(metadata.type()));

//  spectrum_detectors_.clear();
//  for (auto &q: metadata.detectors)
//    spectrum_detectors_.add_a(q);
////  det_table_model_.update();
//
//  Setting pat = metadata.get_attribute("pattern_add");
//  ui->spinDets->setValue(pat.pattern().gates().size());

  QString descr = QS(metadata.type_description())
//        + "   dimensions=" + QString::number(md.dimensions())
      + "\n";
  ui->labelDescription->setText(descr);

  enforce_everything();

  open_close_locks();
}

void ConsumerDialog::enforce_everything()
{
  auto metadata = consumer_->metadata();
  auto tree = attr_model_.get_tree();
  enforce_streams(tree, stream_manifest_);

  auto tempmeta = consumer_->metadata();
  tempmeta.overwrite_all_attributes(tree);
  metadata.set_attributes(tempmeta.attributes_flat());

  initialize_gui_specific(metadata);

//  consumer_->set_attributes(metadata.attributes());

  for (auto s : metadata.attributes_flat())
    consumer_->set_attribute(s);

  attr_model_.update(consumer_->metadata().attributes());
}

void ConsumerDialog::push_settings()
{
  enforce_everything();
  changed_ = true;

  if (ui->pushExpandAll->isChecked())
    ui->treeAttribs->expandAll();
}

void ConsumerDialog::enforce_streams(DAQuiri::Setting& tree,
                                     DAQuiri::StreamManifest stream_manifest)
{
  //hack to find seetings with "stream" flag
  DAQuiri::Setting tree2 = tree;
  tree2.enable_if_flag(false, "");
  tree2.enable_if_flag(true, "stream");
  tree2.cull_readonly();
  std::set<std::string> selected_streams;
  for (auto s : tree2.branches)
  {
    if (!stream_manifest.count(s.get_text()))
    {
      if (stream_manifest.size())
        s.set_text(stream_manifest.begin()->first);
      else
        s.set_text("");
      tree.set(s);
    }
    selected_streams.insert(s.get_text());
  }

  std::set<std::string> values;
  std::set<std::string> traces;
  std::set<std::string> stats;
  for (auto s : selected_streams)
  {
    for (auto v : stream_manifest[s].event_model.value_names)
      values.insert(v);
    for (auto v : stream_manifest[s].event_model.trace_names)
      traces.insert(v);
    for (const auto& v : stream_manifest[s].stats.branches)
      stats.insert(v.id());
  }

  tree2 = tree;
  tree2.enable_if_flag(false, "");
  tree2.enable_if_flag(true, "event_value");
  tree2.cull_readonly();
  for (auto s : tree2.branches)
  {
    auto name = s.get_text();
    if (values.count(name))
      continue;
    if (values.size())
      s.set_text(*values.begin());
    else
      s.set_text("");
    tree.set(s);
  }

  tree2 = tree;
  tree2.enable_if_flag(false, "");
  tree2.enable_if_flag(true, "event_trace");
  tree2.cull_readonly();
  for (auto& s : tree2.branches)
  {
    auto name = s.get_text();
    if (traces.count(name))
      continue;
    if (traces.size())
      s.set_text(*traces.begin());
    else
      s.set_text("");
    tree.set(s);
  }

  tree2 = tree;
  tree2.enable_if_flag(false, "");
  tree2.enable_if_flag(true, "stat_value");
  tree2.cull_readonly();
  for (auto& s : tree2.branches)
  {
    auto name = s.get_text();
    if (stats.count(name))
      continue;
    if (stats.size())
      s.set_text(*stats.begin());
    else
      s.set_text("");
    tree.set(s);
  }

  attr_delegate_.set_valid_streams(selected_streams);
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

//  if (i < static_cast<int>(consumer_metadata_.detectors.size()))
//  {
//    ui->pushDetEdit->setEnabled(unlocked);
//    ui->pushDetRename->setEnabled(unlocked);
//    ui->pushDetToDB->setEnabled(unlocked);
//    Detector det = consumer_metadata_.detectors[i];
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
  saveSettings();

  if (!changed_)
    reject();
  else
  {
    ConsumerPtr newconsumer =
        ConsumerFactory::singleton().create_from_prototype(consumer_->metadata());

    if (!newconsumer)
    {
      QMessageBox msgBox;
      msgBox.setText("Attributes invalid for this type. Check requirements.");
      msgBox.exec();
      return;
    }
    else
    {
      consumer_ = newconsumer;
      accept();
    }
  }
}

void ConsumerDialog::ask_file(Setting set, QModelIndex index)
{
  auto fd = new QFileDialog(qobject_cast<QWidget*>(parent()), QString("Chose File"),
                            QFileInfo(QS(set.get_text())).dir().absolutePath(),
                            QS(set.metadata().get_string("wildcards","")));
  fd->setOption(QFileDialog::DontUseNativeDialog, true);
  fd->setFileMode(QFileDialog::ExistingFile);
  if (fd->exec() && !fd->selectedFiles().isEmpty()) // && (validateFile(parent, fd->selectedFiles().front(), false))
    attr_model_.setData(index, QVariant::fromValue(fd->selectedFiles().front()), Qt::EditRole);
}

void ConsumerDialog::ask_dir(Setting set, QModelIndex index)
{
  auto fd = new QFileDialog(qobject_cast<QWidget*>(parent()), QString("Chose Directory"),
                            QFileInfo(QS(set.get_text())).dir().absolutePath());
  fd->setOption(QFileDialog::DontUseNativeDialog, true);
  fd->setFileMode(QFileDialog::Directory);
  if (fd->exec() && !fd->selectedFiles().isEmpty()) // && (validateFile(parent, fd->selectedFiles().front(), false))
    attr_model_.setData(index, QVariant::fromValue(fd->selectedFiles().front()), Qt::EditRole);
}

void ConsumerDialog::ask_gradient(QString gname, QModelIndex index)
{
  auto gs = new QPlot::GradientSelector(QPlot::Gradients::defaultGradients(),
                                        gname,
                                        qobject_cast<QWidget*>(parent()));
  gs->setModal(true);
  gs->exec();

  attr_model_.setData(index, gs->selected_gradient(), Qt::EditRole);
}

void ConsumerDialog::on_buttonBox_rejected()
{
  reject();
}

void ConsumerDialog::on_spinDets_valueChanged(int arg1)
{
  ConsumerMetadata metadata;
  if (consumer_)
    metadata = consumer_->metadata();
  Setting pat = metadata.get_attribute("pattern_add");

  metadata.set_det_limit(arg1);
  if (arg1 != static_cast<int>(pat.pattern().gates().size()))
    changed_ = true;

  attr_model_.update(metadata.attributes());
  open_close_locks();
}

void ConsumerDialog::on_comboType_activated(const QString& arg1)
{
  std::list<Setting> old;
  if (consumer_)
    old = consumer_->metadata().attributes_flat();
  ConsumerMetadata metadata =
      ConsumerFactory::singleton().create_prototype(arg1.toStdString());
//  on_spinDets_valueChanged(ui->spinDets->value());
  metadata.set_attributes(old);
  attr_model_.update(metadata.attributes());
  consumer_ = ConsumerFactory::singleton().create_from_prototype(metadata);

  updateData();
}

void ConsumerDialog::initialize_gui_specific(DAQuiri::ConsumerMetadata& md)
{
  Setting col = md.get_attribute("appearance");
  if (col.metadata().has_flag("color"))
  {
    if (col.get_text().empty())
      col.set_text(generateColor().name(QColor::HexArgb).toStdString());
    else
      col.set_text(QColor(QS(col.get_text())).name(QColor::HexArgb).toStdString());
    md.set_attribute(col);
  }
  else if (col.metadata().has_flag("gradient-name"))
  {
    auto name = QS(col.get_text());
    auto dg = QPlot::Gradients::defaultGradients();
    if (col.get_text().empty() || !dg.contains(name))
    {
      col.set_text(dg.names()[0].toStdString());
      md.set_attribute(col);
    }
  }
}

void ConsumerDialog::on_pushExpandAll_toggled(bool checked)
{
  if (checked)
    ui->treeAttribs->expandAll();
  else
    ui->treeAttribs->collapseAll();
}

void ConsumerDialog::loadSettings()
{
  QSettings settings;
  settings.beginGroup("ConsumerDialog");
  ui->pushExpandAll->setChecked(settings.value("settings_expand_all", false).toBool());
}

void ConsumerDialog::saveSettings()
{
  QSettings settings;
  settings.beginGroup("ConsumerDialog");
  settings.setValue("settings_expand_all", ui->pushExpandAll->isChecked());
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
  Q_UNUSED(newDetector)
//  QModelIndexList ixl = ui->tableDetectors->selectionModel()->selectedRows();
//  if (ixl.empty())
//    return;
//  int i = ixl.front().row();

//  if (i < static_cast<int>(consumer_metadata_.detectors.size()))
//  {
//    consumer_metadata_.detectors[i] = newDetector;
//    changed_ = true;

//    spectrum_detectors_.clear();
//    for (auto &q: consumer_metadata_.detectors)
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
//                                       QS(spectrum_detectors_.get(i).name()),
//                                       &ok);
//  if (ok && !text.isEmpty())
//  {
//    if (i < static_cast<int>(consumer_metadata_.detectors.size()))
//    {
//      consumer_metadata_.detectors[i].set_name(text.toStdString());
//      changed_ = true;

//      spectrum_detectors_.clear();
//      for (auto &q: consumer_metadata_.detectors)
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

//  if (i < static_cast<int>(consumer_metadata_.detectors.size()))
//  {
//    Detector newdet = detectors_.get(consumer_metadata_.detectors[i]);
//    consumer_metadata_.detectors[i] = newdet;
//    changed_ = true;

//    spectrum_detectors_.clear();
//    for (auto &q: consumer_metadata_.detectors)
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

//  if (i < static_cast<int>(consumer_metadata_.detectors.size()))
//  {
//    Detector newdet = consumer_metadata_.detectors[i];

//    if (!detectors_.has_a(newdet)) {
//      bool ok;
//      QString text = QInputDialog::getText(this, "New Detector",
//                                           "Detector name:", QLineEdit::Normal,
//                                           QS(newdet.name()),
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
//            if (consumer_metadata_.detectors[i].name() != newdet.name())
//            {
//              consumer_metadata_.detectors[i] = newdet;
//              changed_ = true;
////              updateData();
//            }
//          }
//        } else
//          detectors_.replace(newdet);
//      }
//    } else {
//      QMessageBox::StandardButton reply = QMessageBox::question(this, "Replace existing?",
//                                    "Detector " + QS(newdet.name()) + " already exists. Replace?",
//                                     QMessageBox::Yes|QMessageBox::No);
//      if (reply == QMessageBox::No)
//        return;
//      else
//        detectors_.replace(newdet);
//    }
//  }
}
