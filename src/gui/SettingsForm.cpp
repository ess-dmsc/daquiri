#include "SettingsForm.h"
#include "ui_SettingsForm.h"
//#include "widget_detectors.h"
#include "ProfilesForm.h"
#include "BinaryWidget.h"
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QDir>

#include "json_file.h"

#include "producer_factory.h"
#include <QInputDialog>
#include "Profiles.h"

SettingsForm::SettingsForm(ThreadRunner& thread,
                           Container<Detector>& detectors,
                           QWidget *parent) :
  QWidget(parent),
  runner_thread_(thread),
  detectors_(detectors),
  tree_settings_model_(this),
  table_settings_model_(this),
  ui(new Ui::SettingsForm)
{
  ui->setupUi(this);
  ui->Oscil->setVisible(false);

  this->setWindowTitle("DAQ Settings");

  connect(&runner_thread_, SIGNAL(settingsUpdated(DAQuiri::Setting, std::vector<DAQuiri::Detector>, DAQuiri::ProducerStatus)),
          this, SLOT(update(DAQuiri::Setting, std::vector<DAQuiri::Detector>, DAQuiri::ProducerStatus)));
  connect(&runner_thread_, SIGNAL(bootComplete()), this, SLOT(post_boot()));

  connect(&runner_thread_, SIGNAL(oscilReadOut(DAQuiri::OscilData)),
          ui->Oscil, SLOT(oscil_complete(DAQuiri::OscilData)));

  connect(ui->Oscil, SIGNAL(refresh_oscil()), this, SLOT(refresh_oscil()));

  current_status_ = DAQuiri::ProducerStatus::dead;
  tree_settings_model_.update(settings_tree_);

  tree_settings_view_ = new QTreeView(this);
  ui->tabsSettings->addTab(tree_settings_view_, "Settings tree");
  tree_settings_view_->setModel(&tree_settings_model_);
  tree_settings_view_->setItemDelegate(&tree_delegate_);
  tree_settings_view_->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  tree_delegate_.set_detectors(detectors_);
  connect(&tree_delegate_, SIGNAL(begin_editing()), this, SLOT(begin_editing()));
  connect(&tree_delegate_, SIGNAL(ask_execute(DAQuiri::Setting, QModelIndex)), this,
          SLOT(ask_execute_tree(DAQuiri::Setting, QModelIndex)));
  connect(&tree_delegate_, SIGNAL(ask_binary(DAQuiri::Setting, QModelIndex)),
          this, SLOT(ask_binary_tree(DAQuiri::Setting, QModelIndex)));
  connect(&tree_delegate_, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
          this, SLOT(stop_editing(QWidget*,QAbstractItemDelegate::EndEditHint)));

  table_settings_view_ = new QTableView(this);
  ui->tabsSettings->addTab(table_settings_view_, "Settings table");
  table_settings_view_->setModel(&table_settings_model_);
  table_settings_view_->setItemDelegate(&table_settings_delegate_);
  table_settings_view_->horizontalHeader()->setStretchLastSection(true);
  table_settings_view_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  table_settings_delegate_.set_detectors(detectors_);
  table_settings_model_.update(settings_table_);
  table_settings_view_->show();
  connect(&table_settings_delegate_, SIGNAL(begin_editing()), this, SLOT(begin_editing()));
  connect(&table_settings_delegate_, SIGNAL(ask_execute(DAQuiri::Setting, QModelIndex)),
          this, SLOT(ask_execute_table(DAQuiri::Setting, QModelIndex)));
  connect(&table_settings_delegate_, SIGNAL(ask_binary(DAQuiri::Setting, QModelIndex)),
          this, SLOT(ask_binary_table(DAQuiri::Setting, QModelIndex)));
  connect(&table_settings_delegate_, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
          this, SLOT(stop_editing(QWidget*,QAbstractItemDelegate::EndEditHint)));

  connect(&tree_settings_model_, SIGNAL(tree_changed()),
          this, SLOT(push_settings()));
  connect(&tree_settings_model_, SIGNAL(detector_chosen(int, std::string)),
          this, SLOT(chose_detector(int,std::string)));

  connect(&table_settings_model_, SIGNAL(setting_changed(DAQuiri::Setting)),
          this, SLOT(push_from_table(DAQuiri::Setting)));
  connect(&table_settings_model_, SIGNAL(detector_chosen(int, std::string)),
          this, SLOT(chose_detector(int,std::string)));

  detectorOptions.addAction("Apply saved settings", this, SLOT(apply_detector_presets()));
  detectorOptions.addAction("Edit detector database", this, SLOT(open_detector_DB()));
  ui->toolDetectors->setMenu(&detectorOptions);

  loadSettings();

  QTimer::singleShot(50, this, SLOT(init_profile()));
}

void SettingsForm::exit()
{
  exiting_ = true;
}

void SettingsForm::update(const DAQuiri::Setting &tree,
                          const std::vector<DAQuiri::Detector> &channels,
                          DAQuiri::ProducerStatus status)
{
  bool oscil = (status & DAQuiri::ProducerStatus::can_oscil);
  if (oscil) {
    ui->Oscil->setVisible(true);
    ui->Oscil->updateMenu(channels);
  } else
    ui->Oscil->setVisible(false);

  bool can_run = ((status & DAQuiri::ProducerStatus::can_run) != 0);
  bool can_gain_match = false;
  bool can_optimize = false;
  for (auto &q : settings_table_)
    for (auto & p: q.optimizations())
    {
      if (p.metadata().has_flag("gain"))
        can_gain_match = true;
      if (p.metadata().has_flag("optimize"))
        can_optimize = true;
    }


  //update dets in DB as well?

  if (editing_)
  {
    //    DBG << "<SettingsForm> ignoring update";
    return;
  }

  settings_tree_ = tree;
  settings_table_ = channels;

  //  DBG << "tree received " << settings_tree_.branches.size();

  tree_settings_view_->clearSelection();
  //table_settings_view_->clearSelection();

  tree_settings_model_.update(settings_tree_);
  table_settings_model_.update(settings_table_);

  table_settings_view_->resizeColumnsToContents();
  table_settings_view_->horizontalHeader()->setStretchLastSection(true);
}

void SettingsForm::begin_editing()
{
  editing_ = true;
}

void SettingsForm::stop_editing(QWidget*,QAbstractItemDelegate::EndEditHint)
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
  BinaryWidget *editor = new BinaryWidget(set, qobject_cast<QWidget *> (parent()));
  editor->setModal(true);
  editor->exec();

  if (!set.metadata().has_flag("readonly"))
    tree_settings_model_.setData(index, QVariant::fromValue(editor->get_setting().get_number()), Qt::EditRole);
  editing_ = false;
}

void SettingsForm::ask_binary_table(Setting set, QModelIndex index)
{
  if (set.metadata().enum_map().empty())
    return;

  editing_ = true;
  BinaryWidget *editor = new BinaryWidget(set, qobject_cast<QWidget *> (parent()));
  editor->setModal(true);
  editor->exec();

  if (!set.metadata().has_flag("readonly"))
    table_settings_model_.setData(index, QVariant::fromValue(editor->get_setting().get_number()), Qt::EditRole);
  editing_ = false;
}

void SettingsForm::ask_execute_table(Setting command, QModelIndex index)
{
  editing_ = true;

  QMessageBox *editor = new QMessageBox(qobject_cast<QWidget *> (parent()));
  editor->setText("Run " + QString::fromStdString(command.id()));
  editor->setInformativeText("Will run command: " + QString::fromStdString(command.id()) + "\n Are you sure?");
  editor->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  editor->exec();

  if (editor->standardButton(editor->clickedButton()) == QMessageBox::Yes)
    table_settings_model_.setData(index, QVariant::fromValue(1), Qt::EditRole);
  editing_ = false;
}


void SettingsForm::ask_execute_tree(Setting command, QModelIndex index) {
  editing_ = true;

  QMessageBox *editor = new QMessageBox(qobject_cast<QWidget *> (parent()));
  editor->setText("Run " + QString::fromStdString(command.id()));
  editor->setInformativeText("Will run command: " + QString::fromStdString(command.id()) + "\n Are you sure?");
  editor->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  editor->exec();

  if (editor->standardButton(editor->clickedButton()) == QMessageBox::Yes)
    tree_settings_model_.setData(index, QVariant::fromValue(1), Qt::EditRole);
  editing_ = false;
}

void SettingsForm::push_from_table(Setting setting)
{
  editing_ = false;

  //setting.indices.insert(chan);
  emit toggleIO(false);
  runner_thread_.do_set_setting(setting, Match::id | Match::indices);
}

void SettingsForm::chose_detector(int chan, std::string name)
{
  editing_ = false;
  Detector det = detectors_.get(Detector(name));
  //  DBG << "det " <<  det.name() << " with sets " << det.optimizations().size();

  emit toggleIO(false);
  runner_thread_.do_set_detector(chan, det);
}

void SettingsForm::refresh()
{
  emit toggleIO(false);
  runner_thread_.do_refresh_settings();
}

void SettingsForm::closeEvent(QCloseEvent *event)
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

void SettingsForm::toggle_push(bool enable, DAQuiri::ProducerStatus status)
{
  ui->Oscil->toggle_push(enable, status);

  //  bool online = (status & DAQuiri::ProducerStatus::can_run);

  //busy status?!?!
  bool online = (status & DAQuiri::ProducerStatus::booted);

  ui->pushSettingsRefresh->setEnabled(enable && online);
  ui->pushAddProducer->setEnabled(enable && !online);
  ui->pushRemoveProducer->setEnabled(enable && !online);

  ui->pushChangeProfile->setEnabled(enable);

  if (enable)
  {
    table_settings_view_->setEditTriggers(QAbstractItemView::AllEditTriggers);
    tree_settings_view_->setEditTriggers(QAbstractItemView::AllEditTriggers);
  }
  else
  {
    table_settings_view_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tree_settings_view_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  }

  ui->bootButton->setEnabled(enable);
  //  ui->pushOptimizeAll->setEnabled(enable && (online || offline));

  if (online)
  {
    ui->bootButton->setText("Reset");
    ui->bootButton->setIcon(QIcon(":/icons/oxy/16/start.png"));
  }
  else
  {
    ui->bootButton->setText("Boot");
    ui->bootButton->setIcon(QIcon(":/icons/boot16.png"));
  }

  if ((current_status_ & DAQuiri::ProducerStatus::booted) &&
      !(status & DAQuiri::ProducerStatus::booted))
    chan_settings_to_det_DB();

  current_status_ = status;
}

void SettingsForm::loadSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  ui->checkShowRO->setChecked(settings.value("settings_table_show_readonly", true).toBool());
  on_checkShowRO_clicked();

  if (settings.value("settings_tab_tree", true).toBool())
    ui->tabsSettings->setCurrentWidget(tree_settings_view_);
  else
    ui->tabsSettings->setCurrentWidget(table_settings_view_);
}

void SettingsForm::saveSettings()
{
  QSettings settings;
  if (current_status_ & DAQuiri::ProducerStatus::booted)
    chan_settings_to_det_DB();
  settings.beginGroup("Program");
  settings.setValue("settings_table_show_readonly", ui->checkShowRO->isChecked());
  settings.setValue("settings_tab_tree", (ui->tabsSettings->currentWidget() == tree_settings_view_));
  settings.setValue("boot_on_startup", bool(current_status_ & DAQuiri::ProducerStatus::booted));
}

void SettingsForm::chan_settings_to_det_DB()
{
  for (auto &q : settings_table_)
  {
    if (q.id() == "none")
      continue;
    Detector det = detectors_.get(q);
    if (det != Detector())
    {
      det.add_optimizations(q.optimizations());
      detectors_.replace(det);
    }
  }
}


void SettingsForm::updateDetDB()
{
  tree_delegate_.set_detectors(detectors_);
  table_settings_delegate_.set_detectors(detectors_);

  tree_settings_model_.update(settings_tree_);
  table_settings_model_.update(settings_table_);

  table_settings_view_->horizontalHeader()->setStretchLastSection(true);
  table_settings_view_->resizeColumnsToContents();
}

SettingsForm::~SettingsForm()
{
  delete ui;
}

void SettingsForm::post_boot()
{
  apply_detector_presets(); //make this optional?
}

void SettingsForm::apply_detector_presets()
{
  emit toggleIO(false);

  std::map<int, Detector> update;
  for (size_t i=0; i < settings_table_.size(); ++i)
    if (detectors_.has_a(settings_table_[i]))
      update[i] = detectors_.get(settings_table_[i]);

  runner_thread_.do_set_detectors(update);
}

void SettingsForm::on_pushSettingsRefresh_clicked()
{
  editing_ = false;
  emit toggleIO(false);
  runner_thread_.do_refresh_settings();
}

void SettingsForm::open_detector_DB()
{
  //  WidgetDetectors *det_widget = new WidgetDetectors(this);
  //  det_widget->setData(detectors_);
  //  connect(det_widget, SIGNAL(detectorsUpdated()), this, SLOT(updateDetDB()));
  //  det_widget->exec();
}

void SettingsForm::on_checkShowRO_clicked()
{
  //tree_settings_view_->clearSelection();
  //table_settings_view_->clearSelection();

  table_settings_model_.set_show_read_only(ui->checkShowRO->isChecked());
  table_settings_model_.update(settings_table_);

  tree_settings_model_.set_show_read_only(ui->checkShowRO->isChecked());
  tree_settings_model_.update(settings_tree_);
}

void SettingsForm::on_bootButton_clicked()
{
  if (ui->bootButton->text() == "Boot")
  {
    emit toggleIO(false);
    //    INFO << "Booting system...";

    runner_thread_.do_boot();
  }
  else
  {
    emit toggleIO(false);
    QSettings settings;
    settings.beginGroup("Program");
    settings.setValue("boot_on_startup", false);

    //    INFO << "Shutting down";
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
  QSettings settings;
  settings.beginGroup("Program");
  bool boot = settings.value("boot_on_startup", false).toBool();
  profile_chosen(Profiles::current_profile_name(), boot);
}

void SettingsForm::profile_chosen(QString name, bool boot)
{
  emit toggleIO(false);
  runner_thread_.do_initialize(name, boot);
}

void SettingsForm::refresh_oscil()
{
  emit toggleIO(false);
  runner_thread_.do_oscil();
}

void SettingsForm::on_pushExpandAll_clicked()
{
  tree_settings_view_->expandAll();
}

void SettingsForm::on_pushAddProducer_clicked()
{
  auto& pf = ProducerFactory::singleton();
  QStringList prods;
  for (auto p : pf.types())
    prods.push_back(QString::fromStdString(p));

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
  if (!ok && text.isEmpty())
    return;

  default_settings.set_text(text.toStdString());
  runner_thread_.add_producer(default_settings);
}

void SettingsForm::on_pushRemoveProducer_clicked()
{
  auto idxs = tree_settings_view_->selectionModel()->selectedIndexes();
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
