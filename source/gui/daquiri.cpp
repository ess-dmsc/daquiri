#include <QSettings>
#include <QScrollBar>
#include <QCloseEvent>
#include <QMessageBox>
#include <QLabel>
#include <QToolButton>

#include <utility>
#include <numeric>
#include <cstdint>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "daquiri.h"
#include "ui_daquiri.h"
#include "custom_timer.h"

#include "ListModeForm.h"
#include "ProjectForm.h"

#include <QTimer>
#include "Profiles.h"

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
{
  qRegisterMetaType<DAQuiri::OscilData>("DAQuiri::OscilData");
  qRegisterMetaType<std::vector<DAQuiri::Detector>>("std::vector<DAQuiri::Detector>");
  qRegisterMetaType<DAQuiri::ListData>("DAQuiri::ListData");
  qRegisterMetaType<DAQuiri::Setting>("DAQuiri::Setting");
  qRegisterMetaType<DAQuiri::ProducerStatus>("DAQuiri::ProducerStatus");
  qRegisterMetaType<DAQuiri::ProjectPtr>("DAQuiri::ProjectPtr");
  qRegisterMetaType<boost::posix_time::time_duration>("boost::posix_time::time_duration");

  CustomLogger::initLogger(&log_stream_, "daquiri_%N.log");
  ui->setupUi(this);
  connect(&my_emitter_, SIGNAL(writeLine(QString)), this, SLOT(add_log_text(QString)));

  connect(&runner_thread_,
          SIGNAL(settingsUpdated(DAQuiri::Setting, std::vector<DAQuiri::Detector>, DAQuiri::ProducerStatus)),
          this,
          SLOT(update_settings(DAQuiri::Setting, std::vector<DAQuiri::Detector>, DAQuiri::ProducerStatus)));

  loadSettings();

  connect(ui->tabs, SIGNAL(tabCloseRequested(int)), this, SLOT(close_tab_at(int)));

  QToolButton *tb = new QToolButton();
  tb->setIcon(QIcon(":/icons/oxy/16/filenew.png"));
  tb->setMinimumWidth(35);
  tb->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Ignored);
  tb->setToolTip("New project");
  tb->setAutoRaise(true);
  tb->setPopupMode(QToolButton::InstantPopup);
  tb->setToolButtonStyle(Qt::ToolButtonIconOnly);
  tb->setArrowType(Qt::NoArrow);
  // Add empty, not enabled tab to tabWidget
  ui->tabs->addTab(new QLabel("<center>Open new project by clicking \"+\"</center>"), QString());
  ui->tabs->setTabEnabled(0, false);
  // Add tab button to current tab. Button will be enabled, but tab -- not
  ui->tabs->tabBar()->setTabButton(0, QTabBar::RightSide, tb);

  menu_open_.addAction(QIcon(":/icons/oxy/16/filenew.png"), "DAQ project", this, SLOT(open_project()));
  menu_open_.addAction(QIcon(":/icons/oxy/16/filenew.png"), "Raw list", this, SLOT(open_list()));
  tb->setMenu(&menu_open_);

  connect(ui->tabs->tabBar(), SIGNAL(tabMoved(int,int)), this, SLOT(tabs_moved(int,int)));
  connect(ui->tabs, SIGNAL(currentChanged(int)), this, SLOT(tab_changed(int)));

  main_tab_ = new SettingsForm(runner_thread_, detectors_, this);
  ui->tabs->addTab(main_tab_, "DAQ");
//  ui->tabs->addTab(main_tab_, main_tab_->windowTitle());
  ui->tabs->setTabIcon(ui->tabs->count() - 1, QIcon(":/icons/oxy/16/applications_systemg.png"));
  connect(main_tab_, SIGNAL(toggleIO(bool)), this, SLOT(toggleIO(bool)));
  connect(this, SIGNAL(toggle_push(bool,DAQuiri::ProducerStatus)),
          main_tab_, SLOT(toggle_push(bool,DAQuiri::ProducerStatus)));
//  connect(this, SIGNAL(settings_changed()), main_tab_, SLOT(refresh()));
//  connect(this, SIGNAL(update_dets()), main_tab_, SLOT(updateDetDB()));

  if (!Profiles::has_settings_dir())
    initialize_settings_dir();

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
      /*for (int i = ui->tabs->count() - 1; i >= 0; --i)
        if (ui->tabs->widget(i) != main_tab_)
          ui->tabs->widget(i)->exit();*/

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

void daquiri::update_settings(DAQuiri::Setting sets,
                              std::vector<DAQuiri::Detector> /*channels*/,
                              DAQuiri::ProducerStatus status)
{
  engine_status_ = status;
  auto description = sets.find({"Profile description"}, Match::id);
  profile_description_ = QString::fromStdString(description.get_text());
  if (profile_description_.isEmpty())
    profile_description_ = Profiles::current_profile_name();

//  current_dets_ = channels;
  toggleIO(true);

  if ((status & DAQuiri::ProducerStatus::can_run) &&
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

  if (enable && (engine_status_ & DAQuiri::ProducerStatus::running))
    name += " (Running)";
  else if (enable && (engine_status_ & DAQuiri::ProducerStatus::booted))
    name += " (Online)";
  else if (enable)
    name += " (Offline)";
  else
    name += " (Busy)";

  setWindowTitle(name);

  for (int i = 0; i < ui->tabs->count(); ++i)
    if (ui->tabs->widget(i) != main_tab_)
      ui->tabs->setTabText(i, ui->tabs->widget(i)->windowTitle());

  emit toggle_push(enable, engine_status_);
}

void daquiri::loadSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  QRect myrect = settings.value("position",QRect(20,20,1234,650)).toRect();
  ui->splitter->restoreState(settings.value("splitter").toByteArray());
  setGeometry(myrect);
}

void daquiri::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("position", this->geometry());
  settings.setValue("splitter", ui->splitter->saveState());
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
  connect(this, SIGNAL(toggle_push(bool,DAQuiri::ProducerStatus)),
          newListForm, SLOT(toggle_push(bool,DAQuiri::ProducerStatus)));

  ui->tabs->setCurrentWidget(newListForm);

  reorder_tabs();

  emit toggle_push(gui_enabled_, engine_status_);
}

void daquiri::open_new_proj()
{
  open_project(nullptr, start_daq_);
  start_daq_ = false;
}

void daquiri::open_project(DAQuiri::ProjectPtr proj, bool start)
{
  ProjectForm *newSpectraForm = new ProjectForm(runner_thread_, detectors_,
                                              current_dets_,
                                              proj, this);
  connect(newSpectraForm, SIGNAL(requestClose(QWidget*)), this, SLOT(close_tab_widget(QWidget*)));

  connect(newSpectraForm, SIGNAL(toggleIO(bool)), this, SLOT(toggleIO(bool)));
  connect(this, SIGNAL(toggle_push(bool,DAQuiri::ProducerStatus)),
          newSpectraForm, SLOT(toggle_push(bool,DAQuiri::ProducerStatus)));

  add_closable_tab(newSpectraForm);
  ui->tabs->setCurrentWidget(newSpectraForm);
  reorder_tabs();

  newSpectraForm->toggle_push(true, engine_status_);
  if (start)
    QTimer::singleShot(500, newSpectraForm, SLOT(start_DAQ()));
}

void daquiri::initialize_settings_dir()
{

}
