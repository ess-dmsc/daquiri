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

#include "project.h"

#include "ListModeForm.h"
#include "ProjectForm.h"

//#include "qt_util.h"

#include "consumer_factory.h"
#include "spectrum_events_1D.h"
#include "spectrum_events_2D.h"
#include "spectrum_values_2D.h"
static ConsumerRegistrar<Spectrum1DEvent> cons1("1DEvent");
static ConsumerRegistrar<Spectrum2DEvent> cons2("2DEvent");
static ConsumerRegistrar<Image2D> cons3("Image2D");

#include "producer_factory.h"
#include "mock_producer.h"
#include "kafka_producer.h"
#include "dummy_device.h"
static ProducerRegistrar<MockProducer> prod1("MockProducer");
static ProducerRegistrar<KafkaProducer> prod2("KafkaProducer");
static ProducerRegistrar<DummyDevice> prod3("DummyDevice");

daquiri::daquiri(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::daquiri)
  , my_emitter_()
  , log_stream_()
  , text_buffer_(log_stream_, my_emitter_)
{
  detectors_.add(Detector("D1"));
  detectors_.add(Detector("D2"));
  detectors_.add(Detector("D3"));

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

  connect(ui->tabs, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloseRequested(int)));
  ui->statusBar->showMessage("Offline");

  gui_enabled_ = true;

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

  menuOpen.addAction(QIcon(":/icons/oxy/16/filenew.png"), "DAQ project", this, SLOT(openNewProject()));
  menuOpen.addAction(QIcon(":/icons/oxy/16/filenew.png"), "Live list mode", this, SLOT(open_list()));
  tb->setMenu(&menuOpen);

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

  ui->tabs->setCurrentWidget(main_tab_);
  reorder_tabs();
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

void daquiri::tabCloseRequested(int index)
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

void daquiri::update_settings(DAQuiri::Setting /*sets*/,
                              std::vector<DAQuiri::Detector> channels,
                              DAQuiri::ProducerStatus status)
{
  px_status_ = status;
//  current_dets_ = channels;
  toggleIO(true);
}

void daquiri::toggleIO(bool enable)
{
  gui_enabled_ = enable;

  if (enable && (px_status_ & DAQuiri::ProducerStatus::booted))
    ui->statusBar->showMessage("Online");
  else if (enable)
    ui->statusBar->showMessage("Offline");
  else
    ui->statusBar->showMessage("Busy");

  for (int i = 0; i < ui->tabs->count(); ++i)
    if (ui->tabs->widget(i) != main_tab_)
      ui->tabs->setTabText(i, ui->tabs->widget(i)->windowTitle());

  emit toggle_push(enable, px_status_);
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

void daquiri::updateStatusText(QString text)
{
  ui->statusBar->showMessage(text);
}

void daquiri::on_splitter_splitterMoved(int /*pos*/, int /*index*/)
{
  ui->logBox->verticalScrollBar()->setValue(ui->logBox->verticalScrollBar()->maximum());
}

void daquiri::addClosableTab(QWidget* widget, QString tooltip)
{
  CloseTabButton *cb = new CloseTabButton(widget);
  cb->setIcon( QIcon(":/icons/oxy/16/application_exit.png"));
  //  tb->setIconSize(QSize(16, 16));
  cb->setToolTip(tooltip);
  cb->setFlat(true);
  connect(cb, SIGNAL(closeTab(QWidget*)), this, SLOT(closeTab(QWidget*)));
  ui->tabs->addTab(widget, widget->windowTitle());
  ui->tabs->tabBar()->setTabButton(ui->tabs->count()-1, QTabBar::RightSide, cb);
}

void daquiri::closeTab(QWidget* w)
{
  int idx = ui->tabs->indexOf(w);
  tabCloseRequested(idx);
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

bool daquiri::hasTab(QString tofind)
{
  for (int i = 0; i < ui->tabs->count(); ++i)
    if (ui->tabs->tabText(i) == tofind)
      return true;
  return false;
}

void daquiri::open_list()
{
  ListModeForm *newListForm = new ListModeForm(runner_thread_, this);
  addClosableTab(newListForm, "Close");

  connect(newListForm, SIGNAL(toggleIO(bool)), this, SLOT(toggleIO(bool)));
  connect(newListForm, SIGNAL(statusText(QString)), this, SLOT(updateStatusText(QString)));
  connect(this, SIGNAL(toggle_push(bool,DAQuiri::ProducerStatus)),
          newListForm, SLOT(toggle_push(bool,DAQuiri::ProducerStatus)));

  ui->tabs->setCurrentWidget(newListForm);

  reorder_tabs();

  emit toggle_push(gui_enabled_, px_status_);
}

void daquiri::openNewProject()
{
  open_project(nullptr);
}

void daquiri::open_project(DAQuiri::ProjectPtr proj)
{
  ProjectForm *newSpectraForm = new ProjectForm(runner_thread_, detectors_,
                                              current_dets_,
                                              proj, this);
  connect(newSpectraForm, SIGNAL(requestClose(QWidget*)), this, SLOT(closeTab(QWidget*)));

  connect(newSpectraForm, SIGNAL(toggleIO(bool)), this, SLOT(toggleIO(bool)));
  connect(this, SIGNAL(toggle_push(bool,DAQuiri::ProducerStatus)),
          newSpectraForm, SLOT(toggle_push(bool,DAQuiri::ProducerStatus)));

  addClosableTab(newSpectraForm, "Close");
  ui->tabs->setCurrentWidget(newSpectraForm);
  reorder_tabs();

  newSpectraForm->toggle_push(true, px_status_);
}
