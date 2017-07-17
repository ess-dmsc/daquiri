#pragma once

#include <QMainWindow>

#include "custom_logger.h"
#include "qt_boost_logger.h"

#include <QPushButton>
#include <QMenu>

#include "producer.h"

#include "form_system_settings.h"


namespace Ui {
class daquiri;
}

class CloseTabButton : public QPushButton
{
  Q_OBJECT
public:
  CloseTabButton(QWidget* pt) : parent_tab_(pt) {
    connect(this, SIGNAL(clicked(bool)), this, SLOT(closeme(bool)));
  }
private slots:
  void closeme(bool) { emit closeTab(parent_tab_); }
signals:
  void closeTab(QWidget*);
protected:
  QWidget *parent_tab_;
};

class daquiri : public QMainWindow
{
  Q_OBJECT

public:
  explicit daquiri(QWidget *parent = 0);
  ~daquiri();

private:
  Ui::daquiri *ui;

  Container<DAQuiri::Detector>    detectors_;
  ThreadRunner                runner_thread_;
  DAQuiri::ProducerStatus px_status_;

  //connect gui with boost logger framework
  std::stringstream log_stream_;
  LogEmitter        my_emitter_;
  LogStreamBuffer   text_buffer_;

  FormSystemSettings* main_tab_ {nullptr};

  bool gui_enabled_;

  QMenu  menuOpen;

  //helper functions
  void saveSettings();
  void loadSettings();

  void reorder_tabs();

signals:
  void toggle_push(bool, DAQuiri::ProducerStatus);

protected:
  void closeEvent(QCloseEvent*);

private slots:
  void update_settings(Setting,
                       std::vector<DAQuiri::Detector>,
                       DAQuiri::ProducerStatus);
  void toggleIO(bool);
  void updateStatusText(QString);

  void tabCloseRequested(int index);
  void closeTab(QWidget*);

  //logger receiver
  void add_log_text(QString);

  void on_splitter_splitterMoved(int pos, int index);

  bool hasTab(QString);

  void tabs_moved(int, int);
  void addClosableTab(QWidget*, QString);
  void tab_changed(int);

  void open_list();
};

