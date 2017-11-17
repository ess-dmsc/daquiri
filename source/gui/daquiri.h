#pragma once

#include <QMainWindow>

#include "custom_logger.h"
#include "qt_boost_logger.h"

#include <QPushButton>
#include <QMenu>

#include "producer.h"

#include "SettingsForm.h"


namespace Ui {
class daquiri;
}

class close_tab_widgetButton : public QPushButton
{
    Q_OBJECT
  public:
    close_tab_widgetButton(QWidget* pt) : parent_tab_(pt) {
      connect(this, SIGNAL(clicked(bool)), this, SLOT(closeme(bool)));
    }
  private slots:
    void closeme(bool) { emit close_tab_widget(parent_tab_); }
  signals:
    void close_tab_widget(QWidget*);
  protected:
    QWidget *parent_tab_;
};

class daquiri : public QMainWindow
{
    Q_OBJECT

  public:
    explicit daquiri(QWidget *parent = 0,
                     bool open_new_project = false,
                     bool start_daq = false);
    ~daquiri();

  private:
    Ui::daquiri *ui;
    QMenu menu_open_;

    //connect gui with boost logger framework
    std::stringstream log_stream_;
    LogEmitter        my_emitter_;
    LogStreamBuffer   text_buffer_;

    Container<DAQuiri::Detector>   detectors_;
    std::vector<DAQuiri::Detector> current_dets_;
    ThreadRunner                   runner_thread_;
    DAQuiri::ProducerStatus engine_status_;
    QString profile_description_;

    SettingsForm* main_tab_ {nullptr};

    bool gui_enabled_ {true};

    bool open_new_project_ {false};
    bool start_daq_ {false};


  signals:
    void toggle_push(bool, DAQuiri::ProducerStatus);

  protected:
    void closeEvent(QCloseEvent*) Q_DECL_OVERRIDE;

  private slots:
    void update_settings(DAQuiri::Setting,
                         std::vector<DAQuiri::Detector>,
                         DAQuiri::ProducerStatus);
    void toggleIO(bool);

    void close_tab_at(int index);
    void close_tab_widget(QWidget*);

    //logger receiver
    void add_log_text(QString);

    void on_splitter_splitterMoved(int pos, int index);

    void tabs_moved(int, int);
    void tab_changed(int);

    void open_list();
    void open_new_proj();
    void open_project(DAQuiri::ProjectPtr = nullptr, bool start = false);

  private:

    //helper functions
    void saveSettings();
    void loadSettings();
    void reorder_tabs();
    void add_closable_tab(QWidget*);

    void initialize_settings_dir();
};

