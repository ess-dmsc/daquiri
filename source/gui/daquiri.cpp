#include <gui/daquiri.h>
#include "ui_daquiri.h"

#include <gui/Profiles.h>
#include <gui/daq/ListModeForm.h>
#include <gui/daq/ProjectForm.h>
#include <gui/widgets/TabCloseButton.h>
#include <gui/widgets/QFileExtensions.h>
#include <gui/widgets/qt_util.h>

#include <QSettings>
#include <QScrollBar>
#include <QMessageBox>
#include <QInputDialog>
#include <QLabel>
#include <QTimer>
#include <QDir>
#include <QApplication>
#include <date/date.h>

using namespace DAQuiri;

daquiri::daquiri(QWidget *parent,
                 bool open_new_project,
                 bool start_daq)
  : QMainWindow(parent)
  , ui(new Ui::daquiri)
  , log_stream_()
  , my_emitter_()
  , text_buffer_(log_stream_, my_emitter_)
  , open_new_project_(open_new_project)
  , start_daq_(start_daq)
  , server(this)
{
//  detectors_.add(Detector("a"));
//  detectors_.add(Detector("b"));
//  detectors_.add(Detector("c"));

  qRegisterMetaType<DAQuiri::OscilData>("DAQuiri::OscilData");
  qRegisterMetaType<std::vector<Detector>>("std::vector<Detector>");
  qRegisterMetaType<DAQuiri::ListData>("DAQuiri::ListData");
  qRegisterMetaType<DAQuiri::Setting>("DAQuiri::Setting");
  qRegisterMetaType<DAQuiri::ProducerStatus>("DAQuiri::ProducerStatus");
  qRegisterMetaType<DAQuiri::StreamManifest>("DAQuiri::StreamManifest");
  qRegisterMetaType<DAQuiri::ProjectPtr>("DAQuiri::ProjectPtr");
  qRegisterMetaType<hr_duration_t>("hr_duration_t");
  qRegisterMetaType<hr_time_t >("hr_time_t");

  CustomLogger::initLogger(Severity::Informational, &log_stream_, "daquiri.log");
  ui->setupUi(this);
  connect(&my_emitter_, SIGNAL(writeLine(QString)), this, SLOT(add_log_text(QString)));

  server.start_listen(12345);
  connect(&server, SIGNAL(stopDAQ()), this, SLOT(stop_daq()));
  connect(&server, SIGNAL(startNewDAQ(QString)), this, SLOT(start_new_daq(QString)));
  connect(&server, SIGNAL(close_older(uint32_t)), this, SLOT(close_older(uint32_t)));
  connect(&server, SIGNAL(save()), this, SLOT(save()));
  connect(&server, SIGNAL(die()), this, SLOT(die()));

  connect(&runner_thread_,
          SIGNAL(settingsUpdated(DAQuiri::Setting, DAQuiri::ProducerStatus, DAQuiri::StreamManifest)),
          this, SLOT(update_settings(DAQuiri::Setting, DAQuiri::ProducerStatus, DAQuiri::StreamManifest)));

  loadSettings();

  connect(ui->tabs, SIGNAL(tabCloseRequested(int)), this, SLOT(close_tab_at(int)));

  QPushButton *tb = new QPushButton();
  tb->setIcon(QIcon(":/icons/oxy/16/filenew.png"));
  tb->setMinimumWidth(35);
  tb->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Ignored);
  tb->setToolTip("New project");
  tb->setFlat(true);
  connect(tb, SIGNAL(clicked(bool)), this, SLOT(open_project()));

  // Add empty, not enabled tab to tabWidget
  ui->tabs->addTab(new QLabel("<center>Open new project by clicking \"+\"</center>"), QString());
  ui->tabs->setTabEnabled(0, false);
  // Add tab button to current tab. Button will be enabled, but tab -- not
  ui->tabs->tabBar()->setTabButton(0, QTabBar::RightSide, tb);

  connect(ui->tabs->tabBar(), SIGNAL(tabMoved(int,int)), this, SLOT(tabs_moved(int,int)));
  connect(ui->tabs, SIGNAL(currentChanged(int)), this, SLOT(tab_changed(int)));

  main_tab_ = new SettingsForm(runner_thread_, this);
  ui->tabs->addTab(main_tab_, "DAQ");
//  ui->tabs->addTab(main_tab_, main_tab_->windowTitle());
  ui->tabs->setTabIcon(ui->tabs->count() - 1, QIcon(":/icons/oxy/16/applications_systemg.png"));
  connect(main_tab_, SIGNAL(toggleIO(bool)), this, SLOT(toggleIO(bool)));
  connect(this, SIGNAL(toggle_push(bool, DAQuiri::ProducerStatus, DAQuiri::StreamManifest)),
          main_tab_, SLOT(toggle_push(bool, DAQuiri::ProducerStatus, DAQuiri::StreamManifest)));
  connect(main_tab_, SIGNAL(requestList()), this, SLOT(open_list()));
  //  connect(this, SIGNAL(settings_changed()), main_tab_, SLOT(refresh()));
  //  connect(this, SIGNAL(update_dets()), main_tab_, SLOT(updateDetDB()));

  QTimer::singleShot(10, this, SLOT(initialize_settings_dir()));

  if (open_new_project && !start_daq)
    open_project(nullptr, false);
  else
  {
    ui->tabs->setCurrentWidget(main_tab_);
    reorder_tabs();
  }
}

daquiri::~daquiri()
{
  CustomLogger::closeLogger();
  delete ui;
}

void daquiri::closeEvent(QCloseEvent *event)
{
  if (runner_thread_.running())
  {
    int reply = QMessageBox::warning(this, "Ongoing data acquisition operations",
                                     "Terminate?",
                                     QMessageBox::Yes|QMessageBox::Cancel);
    if (reply == QMessageBox::Yes)
    {
      runner_thread_.terminate();
      runner_thread_.wait();
    }
    else
    {
      event->ignore();
      return;
    }
  }
  else
  {
    runner_thread_.terminate();
    runner_thread_.wait();
  }


  for (int i = ui->tabs->count() - 2; i >= 0; --i)
  {
    if (ui->tabs->widget(i) != main_tab_)
    {
      ui->tabs->setCurrentIndex(i);
      if (!ui->tabs->widget(i)->close())
      {
        event->ignore();
        return;
      }
      else
      {
        ui->tabs->removeTab(i);
      }
    }
  }

  if (main_tab_ != nullptr)
  {
    main_tab_->exit();
    main_tab_->close();
  }

  saveSettings();
  event->accept();
}

void daquiri::close_tab_at(int index)
{
  if ((index < 0) || (index >= ui->tabs->count()))
    return;
  ui->tabs->setCurrentIndex(index);
  if (ui->tabs->widget(index)->close())
    ui->tabs->removeTab(index);
}

void daquiri::tab_changed(int index)
{
  if ((index < 0) || (index >= ui->tabs->count()))
    return;
  if (main_tab_ == nullptr)
    return;
  runner_thread_.set_idle_refresh(ui->tabs->widget(index) == main_tab_);
}

void daquiri::add_log_text(QString line)
{
  ui->logBox->append(line);
}

void daquiri::update_settings(Setting sets,
                              ProducerStatus status,
                              StreamManifest manifest)
{
  engine_status_ = status;
  stream_manifest_ = manifest;
  auto description = sets.find({"ProfileDescr"}, Match::id);
  profile_description_ = QS(description.get_text());
  if (profile_description_.isEmpty())
    profile_description_ = Profiles::singleton().current_profile_name();

  toggleIO(true);

  if ((status & ProducerStatus::can_run) &&
      open_new_project_ && start_daq_)
  {
    open_new_project_ = false;
    QTimer::singleShot(100, this, SLOT(open_new_proj()));
  }
}

void daquiri::toggleIO(bool enable)
{
  gui_enabled_ = enable;

  QString name = "daquiri";
  if (!profile_description_.isEmpty())
    name = profile_description_;

  if (enable && (engine_status_ & ProducerStatus::running))
    name += " (Running)";
  else if (enable && (engine_status_ & ProducerStatus::booted))
    name += " (Online)";
  else if (enable)
    name += " (Offline)";
  else
    name += " (Busy)";

  setWindowTitle(name);

  for (int i = 0; i < ui->tabs->count(); ++i)
    if (ui->tabs->widget(i) != main_tab_)
      ui->tabs->setTabText(i, ui->tabs->widget(i)->windowTitle());

  emit toggle_push(enable, engine_status_, stream_manifest_);
}

void daquiri::loadSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  Profiles::singleton().select_profile(
      settings.value("current_profile","").toString(),
      settings.value("auto_boot","").toBool());
  QRect myrect = settings.value("position",QRect(20,20,1234,650)).toRect();
  setGeometry(myrect);
  ui->splitter->restoreState(settings.value("splitter").toByteArray());
}

void daquiri::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("position", this->geometry());
  settings.setValue("splitter", ui->splitter->saveState());
  settings.setValue("current_profile", Profiles::singleton().current_profile_name());
  settings.setValue("auto_boot", Profiles::singleton().auto_boot());
}

void daquiri::on_splitter_splitterMoved(int /*pos*/, int /*index*/)
{
  ui->logBox->verticalScrollBar()->setValue(ui->logBox->verticalScrollBar()->maximum());
}

void daquiri::add_closable_tab(QWidget* widget)
{
  close_tab_widgetButton *cb = new close_tab_widgetButton(widget);
  cb->setIcon( QIcon(":/icons/oxy/16/application_exit.png"));
  //  tb->setIconSize(QSize(16, 16));
  cb->setToolTip("Close");
  cb->setFlat(true);
  connect(cb, SIGNAL(close_tab_widget(QWidget*)), this, SLOT(close_tab_widget(QWidget*)));
  ui->tabs->addTab(widget, widget->windowTitle());
  ui->tabs->tabBar()->setTabButton(ui->tabs->count()-1, QTabBar::RightSide, cb);
}

void daquiri::close_tab_widget(QWidget* w)
{
  int idx = ui->tabs->indexOf(w);
  close_tab_at(idx);
}

void daquiri::reorder_tabs()
{
  for (int i = 0; i < ui->tabs->count(); ++i)
    if (ui->tabs->tabText(i).isEmpty() && (i != (ui->tabs->count() - 1)))
      ui->tabs->tabBar()->moveTab(i, ui->tabs->count() - 1);
}

void daquiri::tabs_moved(int, int)
{
  reorder_tabs();
}

void daquiri::open_list()
{
  ListModeForm *newListForm = new ListModeForm(runner_thread_, this);
  add_closable_tab(newListForm);

  connect(newListForm, SIGNAL(toggleIO(bool)), this, SLOT(toggleIO(bool)));
  connect(this, SIGNAL(toggle_push(bool, DAQuiri::ProducerStatus, DAQuiri::StreamManifest)),
          newListForm, SLOT(toggle_push(bool, DAQuiri::ProducerStatus, DAQuiri::StreamManifest)));

  ui->tabs->setCurrentWidget(newListForm);

  reorder_tabs();

  emit toggle_push(gui_enabled_, engine_status_, stream_manifest_);
}

void daquiri::open_new_proj()
{
  open_project(nullptr, start_daq_);
  start_daq_ = false;
}

void daquiri::open_project(ProjectPtr proj, bool start, QString name)
{
  auto newSpectraForm = new ProjectForm(runner_thread_, proj, name, this);
  connect(newSpectraForm, SIGNAL(requestClose(QWidget*)),
          this, SLOT(close_tab_widget(QWidget*)));

  connect(newSpectraForm, SIGNAL(toggleIO(bool)), this, SLOT(toggleIO(bool)));
  connect(this, SIGNAL(toggle_push(bool, DAQuiri::ProducerStatus, DAQuiri::StreamManifest)),
          newSpectraForm, SLOT(toggle_push(bool, DAQuiri::ProducerStatus, DAQuiri::StreamManifest)));

  add_closable_tab(newSpectraForm);
  ui->tabs->setCurrentWidget(newSpectraForm);
  reorder_tabs();

  newSpectraForm->toggle_push(true, engine_status_, stream_manifest_);
  if (start && (engine_status_ & ProducerStatus::can_run))
    QTimer::singleShot(500, newSpectraForm, SLOT(start_DAQ()));
}

void daquiri::initialize_settings_dir()
{
  if (Profiles::has_settings_dir())
    return;

  if (Profiles::is_valid_settings_dir(Profiles::default_settings_dir()))
  {
    INFO("Found profiles in default settings dir ({}).",
         Profiles::default_settings_dir().toStdString());
    Profiles::select_settings_dir(Profiles::default_settings_dir());
    main_tab_->on_pushChangeProfile_clicked();
    return;
  }

  bool ok {false};

  QString text = QInputDialog::getText(this, tr("First run?"),
                                       tr("Is this your first run of Daquiri?\n"
                                          "Let's create a settings directory.\n"
                                          "How about this?"),
                                       QLineEdit::Normal,
                                       Profiles::default_settings_dir(), &ok);

  if (!ok || text.isEmpty())
    return;

  QDir dir(text);

  bool exists = dir.exists();

  if (!exists)
    exists = dir.mkpath(".");

  if (exists)
  {
    Profiles::select_settings_dir(text);
    int reply = QMessageBox::warning(this, tr("Install defaults?"),
                                     tr("Shall we also copy default configuration profiles\n"
                                        "to your specified directory?"),
                                        QMessageBox::Yes|QMessageBox::Cancel);
    if (reply == QMessageBox::Yes)
    {
      QFileInfo fi(QCoreApplication::applicationFilePath());
      QDir from_path = fi.absoluteDir();
      from_path.cd("../../data");
      copy_dir_recursive(from_path.path(), Profiles::settings_dir(), true);
    }
  }
}

void daquiri::stop_daq()
{
  for (int i = ui->tabs->count() - 1; i >= 0; --i)
  {
    if (ui->tabs->widget(i) == main_tab_)
      continue;
    if (ProjectForm* of = qobject_cast<ProjectForm*>(ui->tabs->widget(i)))
    {
      if (of->running()) {
        INFO("<daquiri> remote command stopping project at i={}", i);
        of->on_pushStop_clicked();
      }
    }
  }
}

void daquiri::save()
{
  for (int i = ui->tabs->count() - 1; i >= 0; --i)
  {
    if (ui->tabs->widget(i) == main_tab_)
      continue;
    if (ProjectForm* of = qobject_cast<ProjectForm*>(ui->tabs->widget(i)))
    {
      INFO("<daquiri> remote command saving project at i={}", i);
      of->save();
    }
  }
}

void daquiri::start_new_daq(QString name)
{
  INFO("<daquiri> remote command starting new project '{}'", name.toStdString());
  if (engine_status_ & ProducerStatus::can_run)
    open_project(nullptr, true, name);
}

void daquiri::close_older(uint32_t mins)
{
  auto now = std::chrono::system_clock::now();
  for (int i = ui->tabs->count() - 1; i >= 0; --i)
  {
    if (ui->tabs->widget(i) == main_tab_)
      continue;
    if (ProjectForm* of = qobject_cast<ProjectForm*>(ui->tabs->widget(i)))
    {
      auto dif = now - of->opened();
      bool eval = ((date::floor<std::chrono::seconds>(dif).count() / 60) >= mins);
      if (eval) {
        INFO("<daquiri> closing older project at i={}", i);
        ui->tabs->setCurrentIndex(i);
        if (ui->tabs->widget(i)->close())
          ui->tabs->removeTab(i);
      }
    }
  }
}

void daquiri::die()
{
  INFO("<daquiri> remote command shutting down");
  closeEvent(new QCloseEvent());
  QApplication::quit();
}
