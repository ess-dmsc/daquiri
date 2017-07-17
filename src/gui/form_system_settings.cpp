#include "form_system_settings.h"
#include "ui_form_system_settings.h"
//#include "widget_detectors.h"
//#include "widget_profiles.h"
#include "binary_checklist.h"
#include <QMessageBox>
#include <QSettings>
#include <QTimer>

FormSystemSettings::FormSystemSettings(ThreadRunner& thread,
                                       Container<Detector>& detectors,
                                       QWidget *parent) :
  QWidget(parent),
  runner_thread_(thread),
  detectors_(detectors),
  tree_settings_model_(this),
  table_settings_model_(this),
  editing_(false),
  exiting(false),
  ui(new Ui::FormSystemSettings)
{
  ui->setupUi(this);
  ui->formOscilloscope->setVisible(false);

  this->setWindowTitle("DAQ Settings");

  connect(&runner_thread_, SIGNAL(settingsUpdated(Setting, std::vector<DAQuiri::Detector>, DAQuiri::ProducerStatus)),
          this, SLOT(update(Setting, std::vector<DAQuiri::Detector>, DAQuiri::ProducerStatus)));
  connect(&runner_thread_, SIGNAL(bootComplete()), this, SLOT(post_boot()));

  connect(&runner_thread_, SIGNAL(oscilReadOut(OscilData)),
          ui->formOscilloscope, SLOT(oscil_complete(OscilData)));

  connect(ui->formOscilloscope, SIGNAL(refresh_oscil()), this, SLOT(refresh_oscil()));

  current_status_ = DAQuiri::ProducerStatus::dead;
  tree_settings_model_.update(dev_settings_);

  viewTreeSettings = new QTreeView(this);
  ui->tabsSettings->addTab(viewTreeSettings, "Settings tree");
  viewTreeSettings->setModel(&tree_settings_model_);
  viewTreeSettings->setItemDelegate(&tree_delegate_);
  viewTreeSettings->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  tree_delegate_.set_detectors(detectors_);
  connect(&tree_delegate_, SIGNAL(begin_editing()), this, SLOT(begin_editing()));
  connect(&tree_delegate_, SIGNAL(ask_execute(Setting, QModelIndex)), this, SLOT(ask_execute_tree(Setting, QModelIndex)));
  connect(&tree_delegate_, SIGNAL(ask_binary(Setting, QModelIndex)), this, SLOT(ask_binary_tree(Setting, QModelIndex)));
  connect(&tree_delegate_, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)), this, SLOT(stop_editing(QWidget*,QAbstractItemDelegate::EndEditHint)));

  viewTableSettings = new QTableView(this);
  ui->tabsSettings->addTab(viewTableSettings, "Settings table");
  viewTableSettings->setModel(&table_settings_model_);
  viewTableSettings->setItemDelegate(&table_settings_delegate_);
  viewTableSettings->horizontalHeader()->setStretchLastSection(true);
  viewTableSettings->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  table_settings_delegate_.set_detectors(detectors_);
  table_settings_model_.update(channels_);
  viewTableSettings->show();
  connect(&table_settings_delegate_, SIGNAL(begin_editing()), this, SLOT(begin_editing()));
  connect(&table_settings_delegate_, SIGNAL(ask_execute(Setting, QModelIndex)), this, SLOT(ask_execute_table(Setting, QModelIndex)));
  connect(&table_settings_delegate_, SIGNAL(ask_binary(Setting, QModelIndex)), this, SLOT(ask_binary_table(Setting, QModelIndex)));
  connect(&table_settings_delegate_, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)), this, SLOT(stop_editing(QWidget*,QAbstractItemDelegate::EndEditHint)));

  connect(&tree_settings_model_, SIGNAL(tree_changed()), this, SLOT(push_settings()));
  connect(&tree_settings_model_, SIGNAL(detector_chosen(int, std::string)), this, SLOT(chose_detector(int,std::string)));

  connect(&table_settings_model_, SIGNAL(setting_changed(Setting)), this, SLOT(push_from_table(Setting)));
  connect(&table_settings_model_, SIGNAL(detector_chosen(int, std::string)), this, SLOT(chose_detector(int,std::string)));

  detectorOptions.addAction("Apply saved settings", this, SLOT(apply_detector_presets()));
  detectorOptions.addAction("Edit detector database", this, SLOT(open_detector_DB()));
  ui->toolDetectors->setMenu(&detectorOptions);

  loadSettings();

//  QSettings settings;
//  settings.beginGroup("Program");
//  QString profile_directory = settings.value("profile_directory", "").toString();

//  if (!profile_directory.isEmpty())
    QTimer::singleShot(50, this, SLOT(profile_chosen()));
//  else
//    QTimer::singleShot(50, this, SLOT(choose_profiles()));
}

void FormSystemSettings::exit() {
  exiting = true;
}

void FormSystemSettings::update(const Setting &tree, const std::vector<DAQuiri::Detector> &channels, DAQuiri::ProducerStatus status) {
  bool oscil = (status & DAQuiri::ProducerStatus::can_oscil);
  if (oscil) {
    ui->formOscilloscope->setVisible(true);
    ui->formOscilloscope->updateMenu(channels);
  } else
    ui->formOscilloscope->setVisible(false);

  bool can_run = ((status & DAQuiri::ProducerStatus::can_run) != 0);
  bool can_gain_match = false;
  bool can_optimize = false;
  for (auto &q : channels_)
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
//    DBG << "<FormSystemSettings> ignoring update";
    return;
  }

  dev_settings_ = tree;
  channels_ = channels;

//  DBG << "tree received " << dev_settings_.branches.size();

  viewTreeSettings->clearSelection();
  //viewTableSettings->clearSelection();

  tree_settings_model_.update(dev_settings_);
  table_settings_model_.update(channels_);

  viewTableSettings->resizeColumnsToContents();
  viewTableSettings->horizontalHeader()->setStretchLastSection(true);
}

void FormSystemSettings::begin_editing() {
  editing_ = true;
}

void FormSystemSettings::stop_editing(QWidget*,QAbstractItemDelegate::EndEditHint) {
  editing_ = false;
}


void FormSystemSettings::push_settings() {
  editing_ = false;
  dev_settings_ = tree_settings_model_.get_tree();

  emit statusText("Updating settings...");
  emit toggleIO(false);
  runner_thread_.do_push_settings(dev_settings_);
}

void FormSystemSettings::ask_binary_tree(Setting set, QModelIndex index)
{
  if (set.metadata().enum_map().empty())
    return;

  editing_ = true;
  BinaryChecklist *editor = new BinaryChecklist(set, qobject_cast<QWidget *> (parent()));
  editor->setModal(true);
  editor->exec();

  if (!set.metadata().has_flag("readonly"))
    tree_settings_model_.setData(index, QVariant::fromValue(editor->get_setting().get_number()), Qt::EditRole);
  editing_ = false;
}

void FormSystemSettings::ask_binary_table(Setting set, QModelIndex index)
{
  if (set.metadata().enum_map().empty())
    return;

  editing_ = true;
  BinaryChecklist *editor = new BinaryChecklist(set, qobject_cast<QWidget *> (parent()));
  editor->setModal(true);
  editor->exec();

  if (!set.metadata().has_flag("readonly"))
    table_settings_model_.setData(index, QVariant::fromValue(editor->get_setting().get_number()), Qt::EditRole);
  editing_ = false;
}

void FormSystemSettings::ask_execute_table(Setting command, QModelIndex index)
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


void FormSystemSettings::ask_execute_tree(Setting command, QModelIndex index) {
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

void FormSystemSettings::push_from_table(Setting setting)
{
  editing_ = false;

  //setting.indices.insert(chan);

  emit statusText("Updating settings...");
  emit toggleIO(false);
  runner_thread_.do_set_setting(setting, Match::id | Match::indices);
}

void FormSystemSettings::chose_detector(int chan, std::string name)
{
  editing_ = false;
  Detector det = detectors_.get(Detector(name));
//  DBG << "det " <<  det.name() << " with sets " << det.optimizations().size();

  emit statusText("Applying detector settings...");
  emit toggleIO(false);
  runner_thread_.do_set_detector(chan, det);
}

void FormSystemSettings::refresh() {

  emit statusText("Updating settings...");
  emit toggleIO(false);
  runner_thread_.do_refresh_settings();
}

void FormSystemSettings::closeEvent(QCloseEvent *event) {
  if (exiting) {
    saveSettings();
    event->accept();
  }
  else
    event->ignore();
  return;
}

void FormSystemSettings::toggle_push(bool enable, DAQuiri::ProducerStatus status) {
  ui->formOscilloscope->toggle_push(enable, status);

//  bool online = (status & DAQuiri::ProducerStatus::can_run);

  //busy status?!?!
  bool online = (status & DAQuiri::ProducerStatus::booted);

  ui->pushSettingsRefresh->setEnabled(enable && online);
  ui->pushChangeProfile->setEnabled(enable);

  if (enable) {
    viewTableSettings->setEditTriggers(QAbstractItemView::AllEditTriggers);
    viewTreeSettings->setEditTriggers(QAbstractItemView::AllEditTriggers);
  } else {
    viewTableSettings->setEditTriggers(QAbstractItemView::NoEditTriggers);
    viewTreeSettings->setEditTriggers(QAbstractItemView::NoEditTriggers);
  }

  ui->bootButton->setEnabled(enable);
//  ui->pushOptimizeAll->setEnabled(enable && (online || offline));

  if (online) {
    ui->bootButton->setText("Reset system");
    ui->bootButton->setIcon(QIcon(":/icons/oxy/16/start.png"));
  } else {
    ui->bootButton->setText("Boot system");
    ui->bootButton->setIcon(QIcon(":/icons/boot16.png"));
  }

  if ((current_status_ & DAQuiri::ProducerStatus::booted) &&
      !(status & DAQuiri::ProducerStatus::booted))
    chan_settings_to_det_DB();

  current_status_ = status;
}

void FormSystemSettings::loadSettings() {
  QSettings settings;
  settings.beginGroup("Program");
  ui->checkShowRO->setChecked(settings.value("settings_table_show_readonly", true).toBool());
  on_checkShowRO_clicked();

  if (settings.value("settings_tab_tree", true).toBool())
    ui->tabsSettings->setCurrentWidget(viewTreeSettings);
  else
    ui->tabsSettings->setCurrentWidget(viewTableSettings);
}

void FormSystemSettings::saveSettings()
{
  QSettings settings;
  if (current_status_ & DAQuiri::ProducerStatus::booted)
    chan_settings_to_det_DB();
  settings.beginGroup("Program");
  settings.setValue("settings_table_show_readonly", ui->checkShowRO->isChecked());
  settings.setValue("settings_tab_tree", (ui->tabsSettings->currentWidget() == viewTreeSettings));
  settings.setValue("boot_on_startup", bool(current_status_ & DAQuiri::ProducerStatus::booted));
}

void FormSystemSettings::chan_settings_to_det_DB()
{
  for (auto &q : channels_)
  {
    if (q.name() == "none")
      continue;
    Detector det = detectors_.get(q);
    if (det != Detector())
    {
      det.add_optimizations(q.optimizations());
      detectors_.replace(det);
    }
  }
}


void FormSystemSettings::updateDetDB()
{
  tree_delegate_.set_detectors(detectors_);
  table_settings_delegate_.set_detectors(detectors_);

  tree_settings_model_.update(dev_settings_);
  table_settings_model_.update(channels_);

  viewTableSettings->horizontalHeader()->setStretchLastSection(true);
  viewTableSettings->resizeColumnsToContents();
}

FormSystemSettings::~FormSystemSettings()
{
  delete ui;
}

void FormSystemSettings::post_boot()
{
  apply_detector_presets(); //make this optional?
}


void FormSystemSettings::apply_detector_presets()
{
  emit statusText("Applying detector optimizations...");
  emit toggleIO(false);

  std::map<int, Detector> update;
  for (size_t i=0; i < channels_.size(); ++i)
    if (detectors_.has_a(channels_[i]))
      update[i] = detectors_.get(channels_[i]);
    
  runner_thread_.do_set_detectors(update);
}

void FormSystemSettings::on_pushSettingsRefresh_clicked()
{
  editing_ = false;
  emit statusText("Refreshing settings_...");
  emit toggleIO(false);
  runner_thread_.do_refresh_settings();
}

void FormSystemSettings::open_detector_DB()
{
//  WidgetDetectors *det_widget = new WidgetDetectors(this);
//  det_widget->setData(detectors_);
//  connect(det_widget, SIGNAL(detectorsUpdated()), this, SLOT(updateDetDB()));
//  det_widget->exec();
}

void FormSystemSettings::on_checkShowRO_clicked()
{
  //viewTreeSettings->clearSelection();
  //viewTableSettings->clearSelection();

  table_settings_model_.set_show_read_only(ui->checkShowRO->isChecked());
  table_settings_model_.update(channels_);

  tree_settings_model_.set_show_read_only(ui->checkShowRO->isChecked());
  tree_settings_model_.update(dev_settings_);
}

void FormSystemSettings::on_bootButton_clicked()
{
  if (ui->bootButton->text() == "Boot system")
  {
    emit toggleIO(false);
    emit statusText("Booting...");
//    LINFO << "Booting system...";

    runner_thread_.do_boot();
  }
  else
  {
    emit toggleIO(false);
    emit statusText("Shutting down...");

//    LINFO << "Shutting down";
    runner_thread_.do_shutdown();
  }
}

void FormSystemSettings::on_spinRefreshFrequency_valueChanged(int arg1)
{
  runner_thread_.set_idle_refresh_frequency(arg1);
}

void FormSystemSettings::on_pushChangeProfile_clicked()
{
  choose_profiles();
}

void FormSystemSettings::choose_profiles()
{
//  WidgetProfiles *profiles = new WidgetProfiles(this);
//  connect(profiles, SIGNAL(profileChosen()), this, SLOT(profile_chosen()));
//  profiles->exec();
}

void FormSystemSettings::profile_chosen()
{
  emit toggleIO(false);
  runner_thread_.do_initialize();
}

void FormSystemSettings::refresh_oscil()
{
  emit statusText("Getting traces...");
  emit toggleIO(false);
  runner_thread_.do_oscil();
}

void FormSystemSettings::on_pushExpandAll_clicked()
{
  viewTreeSettings->expandAll();
}