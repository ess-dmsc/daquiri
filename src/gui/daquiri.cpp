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

//#include "qt_util.h"

daquiri::daquiri(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::daquiri),
  my_emitter_(),
  log_stream_(),
  text_buffer_(log_stream_, my_emitter_)
{
  qRegisterMetaType<DAQuiri::ListData>("DAQuiri::ListData");
  qRegisterMetaType<DAQuiri::Setting>("DAQuiri::Setting");
  qRegisterMetaType<DAQuiri::ProducerStatus>("DAQuiri::ProducerStatus");
  qRegisterMetaType<DAQuiri::ProjectPtr>("DAQuiri::ProjectPtr");
  qRegisterMetaType<boost::posix_time::time_duration>("boost::posix_time::time_duration");

  CustomLogger::initLogger(&log_stream_, "daquiri_%N.log");
  ui->setupUi(this);
  connect(&my_emitter_, SIGNAL(writeLine(QString)), this, SLOT(add_log_text(QString)));

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

  tb->setMenu(&menuOpen);

  connect(ui->tabs->tabBar(), SIGNAL(tabMoved(int,int)), this, SLOT(tabs_moved(int,int)));
  connect(ui->tabs, SIGNAL(currentChanged(int)), this, SLOT(tab_changed(int)));
}

daquiri::~daquiri()
{
  CustomLogger::closeLogger();
  delete ui;
}

void daquiri::closeEvent(QCloseEvent *event)
{
  for (int i = ui->tabs->count() - 2; i >= 0; --i)
  {
    ui->tabs->setCurrentIndex(i);
    if (!ui->tabs->widget(i)->close())
    {
      event->ignore();
      return;
    } else {
      ui->tabs->removeTab(i);
    }
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
}

void daquiri::add_log_text(QString line)
{
  ui->logBox->append(line);
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
