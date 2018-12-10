#include <gui/SettingsForm.h>
#include "ui_SettingsForm.h"

#include <gui/ProfilesForm.h>
#include <gui/Profiles.h>
#include <gui/widgets/BinaryWidget.h>
#include <gui/widgets/qt_util.h>

#include <core/util/json_file.h>
#include <core/producer_factory.h>

#include <QPlot/GradientSelector.h>
#include <QMessageBox>
#include <QInputDialog>
#include <QSettings>
#include <QTimer>
#include <QDir>

using namespace DAQuiri;

SettingsForm::SettingsForm(ThreadRunner& thread,
                           QWidget* parent)
    : QWidget(parent)
      , ui(new Ui::SettingsForm)
      , runner_thread_(thread)
      , tree_settings_model_(this)
      , tree_delegate_(this)
{
  ui->setupUi(this);

  this->setWindowTitle("DAQ Settings");

  connect(&runner_thread_,
          SIGNAL(settingsUpdated(DAQuiri::Setting, DAQuiri::ProducerStatus, DAQuiri::StreamManifest)),
          this, SLOT(update(DAQuiri::Setting, DAQuiri::ProducerStatus, DAQuiri::StreamManifest)));

  current_status_ = ProducerStatus::dead;
  tree_settings_model_.update(settings_tree_);

  ui->treeViewSettings->setModel(&tree_settings_model_);
  ui->treeViewSettings->setItemDelegate(&tree_delegate_);
  ui->treeViewSettings->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  connect(&tree_delegate_, SIGNAL(begin_editing()), this, SLOT(begin_editing()));
  connect(&tree_delegate_, SIGNAL(ask_execute(DAQuiri::Setting, QModelIndex)), this,
          SLOT(ask_execute_tree(DAQuiri::Setting, QModelIndex)));
  connect(&tree_delegate_, SIGNAL(ask_binary(DAQuiri::Setting, QModelIndex)),
          this, SLOT(ask_binary_tree(DAQuiri::Setting, QModelIndex)));
  connect(&tree_delegate_, SIGNAL(ask_file(DAQuiri::Setting, QModelIndex)),
          this, SLOT(ask_file_tree(DAQuiri::Setting, QModelIndex)));
  connect(&tree_delegate_, SIGNAL(ask_dir(DAQuiri::Setting, QModelIndex)),
          this, SLOT(ask_dir_tree(DAQuiri::Setting, QModelIndex)));
  connect(&tree_delegate_, SIGNAL(ask_gradient(QString, QModelIndex)),
          this, SLOT(ask_gradient_tree(QString, QModelIndex)));
  connect(&tree_delegate_, SIGNAL(closeEditor(QWidget * , QAbstractItemDelegate::EndEditHint)),
          this, SLOT(stop_editing(QWidget * , QAbstractItemDelegate::EndEditHint)));

  connect(&tree_settings_model_, SIGNAL(tree_changed()),
          this, SLOT(push_settings()));

  loadSettings();

  QTimer::singleShot(50, this, SLOT(init_profile()));
}

void SettingsForm::exit()
{
  exiting_ = true;
}

void SettingsForm::update(const Setting& tree,
                          ProducerStatus status,
                          StreamManifest manifest)
{
  Q_UNUSED(status)
  Q_UNUSED(manifest)
//  bool can_run = ((status & ProducerStatus::can_run) != 0);
//  bool can_gain_match = false;
//  bool can_optimize = false;

  //update dets in DB as well?

  if (editing_)
  {
    //    DBG( "<SettingsForm> ignoring update";
    return;
  }

  settings_tree_ = tree;
  //  DBG( "tree received " << settings_tree_.branches.size();

  ui->treeViewSettings->clearSelection();
  tree_settings_model_.update(settings_tree_);
  if (ui->pushExpandAll->isChecked())
    ui->treeViewSettings->expandAll();
}

void SettingsForm::begin_editing()
{
  editing_ = true;
}

void SettingsForm::stop_editing(QWidget*, QAbstractItemDelegate::EndEditHint)
{
  editing_ = false;
}

void SettingsForm::push_settings()
{
  editing_ = false;
  settings_tree_ = tree_settings_model_.get_tree();
  emit toggleIO(false);
  runner_thread_.do_push_settings(settings_tree_);
}

void SettingsForm::ask_binary_tree(Setting set, QModelIndex index)
{
  if (set.metadata().enum_map().empty())
    return;

  editing_ = true;
  BinaryWidget* editor = new BinaryWidget(set, qobject_cast<QWidget*>(parent()));
  editor->setModal(true);
  editor->exec();

  if (!set.metadata().has_flag("readonly"))
    tree_settings_model_.setData(index, QVariant::fromValue(editor->get_setting().get_int()), Qt::EditRole);
  editing_ = false;
}

void SettingsForm::ask_execute_tree(Setting command, QModelIndex index)
{
  editing_ = true;

  QString txt = QS(command.metadata().get_string("command_name", ""));
  if (txt.isEmpty())
    txt = QS(command.id());

  QMessageBox* editor = new QMessageBox(qobject_cast<QWidget*>(parent()));
  editor->setText(txt + "?");
  editor->setInformativeText("Run command: \"" + txt + "\"\n Are you sure?");
  editor->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  editor->exec();

  if (editor->standardButton(editor->clickedButton()) == QMessageBox::Yes)
    tree_settings_model_.setData(index, QVariant::fromValue(1), Qt::EditRole);
  editing_ = false;
}

void SettingsForm::ask_file_tree(Setting set, QModelIndex index)
{
  editing_ = true;

  auto fd = new QFileDialog(qobject_cast<QWidget*>(parent()), QString("Chose File"),
                            QFileInfo(QS(set.get_text())).dir().absolutePath(),
                            QS(set.metadata().get_string("wildcards","")));
  fd->setOption(QFileDialog::DontUseNativeDialog, true);
  fd->setFileMode(QFileDialog::ExistingFile);
  if (fd->exec() && !fd->selectedFiles().isEmpty()) // && (validateFile(parent, fd->selectedFiles().front(), false))
    tree_settings_model_.setData(index, QVariant::fromValue(fd->selectedFiles().front()), Qt::EditRole);
  editing_ = false;
}

void SettingsForm::ask_dir_tree(Setting set, QModelIndex index)
{
  editing_ = true;

  auto fd = new QFileDialog(qobject_cast<QWidget*>(parent()), QString("Chose Directory"),
                            QFileInfo(QS(set.get_text())).dir().absolutePath());
  fd->setOption(QFileDialog::DontUseNativeDialog, true);
  fd->setFileMode(QFileDialog::Directory);
  if (fd->exec() && !fd->selectedFiles().isEmpty()) // && (validateFile(parent, fd->selectedFiles().front(), false))
    tree_settings_model_.setData(index, QVariant::fromValue(fd->selectedFiles().front()), Qt::EditRole);
  editing_ = false;
}

void SettingsForm::ask_gradient_tree(QString gname, QModelIndex index)
{
  editing_ = true;

  auto gs = new QPlot::GradientSelector(QPlot::Gradients::defaultGradients(),
                                        gname,
                                        qobject_cast<QWidget*>(parent()));
  gs->setModal(true);
  gs->exec();

  tree_settings_model_.setData(index, gs->selected_gradient(), Qt::EditRole);

  editing_ = false;
}

void SettingsForm::refresh()
{
  emit toggleIO(false);
  runner_thread_.do_refresh_settings();
}

void SettingsForm::closeEvent(QCloseEvent* event)
{
  if (exiting_)
  {
    saveSettings();
    event->accept();
  }
  else
    event->ignore();
  return;
}

void SettingsForm::toggle_push(bool enable, ProducerStatus status, StreamManifest manifest)
{
  Q_UNUSED(manifest);
  //  bool online = (status & ProducerStatus::can_run);

  //busy status?!?!
  bool online = (status & ProducerStatus::booted);

  ui->pushSettingsRefresh->setEnabled(enable && online);
  ui->pushRequestList->setEnabled(enable && online);
  ui->pushAddProducer->setEnabled(enable && !online);
  ui->pushRemoveProducer->setEnabled(enable && !online);

  ui->pushChangeProfile->setEnabled(enable);

  if (enable)
  {
    ui->treeViewSettings->setEditTriggers(QAbstractItemView::AllEditTriggers);
  }
  else
  {
    ui->treeViewSettings->setEditTriggers(QAbstractItemView::NoEditTriggers);
  }

  ui->bootButton->setEnabled(enable);
  //  ui->pushOptimizeAll->setEnabled(enable && (online || offline));

  if (online)
  {
    ui->bootButton->setText("Reset");
    ui->bootButton->setToolTip("Reset");
    ui->bootButton->setIcon(QIcon(":/icons/oxy/16/start.png"));
  }
  else
  {
    ui->bootButton->setText("Boot");
    ui->bootButton->setToolTip("Reset");
    ui->bootButton->setIcon(QIcon(":/icons/boot16.png"));
  }

  current_status_ = status;
}

void SettingsForm::loadSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  ui->checkShowRO->setChecked(settings.value("settings_show_readonly", true).toBool());
  ui->pushExpandAll->setChecked(settings.value("settings_expand_all", false).toBool());
  on_checkShowRO_clicked();
}

void SettingsForm::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("settings_show_readonly", ui->checkShowRO->isChecked());
  settings.setValue("settings_expand_all", ui->pushExpandAll->isChecked());
}

void SettingsForm::updateDetDB()
{
  tree_settings_model_.update(settings_tree_);
}

SettingsForm::~SettingsForm()
{
  delete ui;
}

void SettingsForm::on_pushSettingsRefresh_clicked()
{
  editing_ = false;
  emit toggleIO(false);
  runner_thread_.do_refresh_settings();
}

void SettingsForm::on_checkShowRO_clicked()
{
  //ui->treeViewSettings->clearSelection();

  tree_settings_model_.set_show_read_only(ui->checkShowRO->isChecked());
  tree_settings_model_.update(settings_tree_);
}

void SettingsForm::on_bootButton_clicked()
{
  if (ui->bootButton->text() == "Boot")
  {
    Profiles::singleton().auto_boot(true);
    emit toggleIO(false);
    runner_thread_.do_boot();
  }
  else
  {
    Profiles::singleton().auto_boot(false);
    emit toggleIO(false);
    runner_thread_.do_shutdown();
  }
}

void SettingsForm::on_spinRefreshFrequency_valueChanged(int arg1)
{
  runner_thread_.set_idle_refresh_frequency(arg1);
}

void SettingsForm::on_pushChangeProfile_clicked()
{
  ProfilesForm* profiles = new ProfilesForm(this);
  connect(profiles, SIGNAL(profileChosen(QString, bool)),
          this, SLOT(profile_chosen(QString, bool)));
  profiles->exec();
}

void SettingsForm::init_profile()
{
  profile_chosen(Profiles::singleton().current_profile_name(), Profiles::singleton().auto_boot());
}

void SettingsForm::profile_chosen(QString name, bool boot)
{
  emit toggleIO(false);
  runner_thread_.do_initialize(name, boot);
}

void SettingsForm::on_pushExpandAll_toggled(bool checked)
{
  if (checked)
    ui->treeViewSettings->expandAll();
  else
    ui->treeViewSettings->collapseAll();
}

void SettingsForm::on_pushAddProducer_clicked()
{
  auto& pf = ProducerFactory::singleton();
  QStringList prods;
  for (auto p : pf.types())
    prods.push_back(QS(p));

  QInputDialog id(this);
  id.setOptions(QInputDialog::UseListViewForComboBoxItems);
  id.setComboBoxItems(prods);
  id.setLabelText("Producer type: ");
  id.setWindowTitle("Add producer");
  int ret = id.exec();

  if (ret != QDialog::Accepted)
    return;

  auto default_settings = pf.default_settings(id.textValue().toStdString());
  if (!default_settings)
    return;

  bool ok;
  QString text = QInputDialog::getText(this, tr("Producer name"),
                                       tr("Specify unique name for producer:"),
                                       QLineEdit::Normal, "", &ok);
  if (!ok || text.isEmpty())
  {
    //Say something

    return;
  }

  default_settings.set_text(text.toStdString());
  runner_thread_.add_producer(default_settings);
}

void SettingsForm::on_pushRemoveProducer_clicked()
{
  auto idxs = ui->treeViewSettings->selectionModel()->selectedIndexes();
  for (auto ixl : idxs)
    if (ixl.data(Qt::EditRole).canConvert<Setting>())
    {
      Setting set = qvariant_cast<Setting>(ixl.data(Qt::EditRole));
      if (set.is(SettingType::stem) && set.metadata().has_flag("producer"))
      {
        runner_thread_.remove_producer(set);
      }
    }
}

void SettingsForm::on_pushRequestList_clicked()
{
  emit requestList();
}
