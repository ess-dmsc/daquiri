#pragma once

#include <QDialog>
#include <boost/date_time.hpp>
#include "qt_util.h"


namespace Ui {
class TimeDurationWidget;
}

class TimeDurationWidget : public QWidget
{
  Q_OBJECT

public:
  explicit TimeDurationWidget(QWidget *parent = 0);
  ~TimeDurationWidget();

  uint64_t total_seconds();
  void set_total_seconds(uint64_t secs);

  void set_duration(boost::posix_time::time_duration duration);
  boost::posix_time::time_duration get_duration() const;

  void set_us_enabled(bool use);

signals:
  void editingFinished();
  void valueChanged();

private slots:
  void on_spinM_valueChanged(int);
  void on_spinH_valueChanged(int);
  void on_spinS_valueChanged(int);
  void on_spin_ms_valueChanged(int arg1);

  void on_spinDays_editingFinished();
  void on_spinH_editingFinished();
  void on_spinM_editingFinished();
  void on_spinS_editingFinished();
  void on_spin_ms_editingFinished();

private:
  Ui::TimeDurationWidget *ui;
  bool us_enabled_;

};
