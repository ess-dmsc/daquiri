#include "TimeDurationWidget.h"
#include "ui_TimeDurationWidget.h"
#include <core/util/custom_logger.h>

#include <date/date.h>


#define FACTOR_us 1000000
#define FACTOR_min 60
#define FACTOR_h 60
#define FACTOR_day 24

TimeDurationWidget::TimeDurationWidget(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::TimeDurationWidget)
  , us_enabled_(true)
{
  ui->setupUi(this);
}

TimeDurationWidget::~TimeDurationWidget()
{
  delete ui;
}

void TimeDurationWidget::set_us_enabled(bool use)
{
  us_enabled_ = use;
  ui->label_us->setVisible(use);
  ui->spin_ms->setVisible(use);
}

uint64_t TimeDurationWidget::total_seconds()
{
  return (60 * (60 * (ui->spinDays->value() * 24 + ui->spinH->value()) + ui->spinM->value()) + ui->spinS->value());
}

void TimeDurationWidget::set_total_seconds(uint64_t secs)
{
  ui->spinS->setValue(secs % 60);
  uint64_t total_minutes = secs / 60;
  ui->spinM->setValue(total_minutes % 60);
  uint64_t total_hours = total_minutes / 60;
  ui->spinH->setValue(total_hours % 24);
  ui->spinDays->setValue(total_hours / 24);
}

// \todo fix this
void TimeDurationWidget::set_duration(hr_duration_t duration)
{
  INFO("Set duration {}", to_simple(duration));
  using namespace date;
  uint64_t total_ms = static_cast<uint64_t>(round<std::chrono::microseconds>(duration).count());
  ui->spin_ms->setValue(total_ms % FACTOR_us);
  uint64_t total_seconds = total_ms / FACTOR_us;
  ui->spinS->setValue(total_seconds % FACTOR_min);
  uint64_t total_minutes = total_seconds / FACTOR_min;
  ui->spinM->setValue(total_minutes % FACTOR_h);
  uint64_t total_hours = total_minutes / FACTOR_h;
  ui->spinH->setValue(total_hours % FACTOR_day);
  ui->spinDays->setValue(total_hours / FACTOR_day);
}

hr_duration_t TimeDurationWidget::get_duration() const
{
  uint64_t time = FACTOR_day * ui->spinDays->value();
  time += ui->spinH->value();
  time *= FACTOR_h;
  time += ui->spinM->value();
  time *= FACTOR_min;
  time += ui->spinS->value();
  time *= FACTOR_us;
  time += ui->spin_ms->value();
  hr_duration_t ret = std::chrono::microseconds(time);
  INFO("Ret duration {}", to_simple(ret));
  return ret;
}


void TimeDurationWidget::on_spin_ms_valueChanged(int new_value)
{
  if (new_value >= FACTOR_us)
  {
    ui->spinS->setValue(ui->spinS->value() + new_value / FACTOR_us);
    if (ui->spinDays->value() != ui->spinDays->maximum())
      ui->spin_ms->setValue(new_value % FACTOR_us);
    else
      ui->spin_ms->setValue(0);
  }
  else if (new_value < 0)
  {
    if ((ui->spinS->value() > 0) ||
        (ui->spinM->value() > 0) ||
        (ui->spinH->value() > 0)  ||
        (ui->spinDays->value() > 0))
    {
      ui->spinS->setValue(ui->spinS->value() - 1);
      ui->spin_ms->setValue(FACTOR_us + new_value);
    }
    else
    {
      ui->spin_ms->setValue(0);
    }
  }
  emit valueChanged();
}


void TimeDurationWidget::on_spinS_valueChanged(int new_value)
{
  if (new_value >= FACTOR_min)
  {
    ui->spinM->setValue(ui->spinM->value() + new_value / FACTOR_min);
    if (ui->spinDays->value() != ui->spinDays->maximum())
      ui->spinS->setValue(new_value % FACTOR_min);
    else
      ui->spinS->setValue(0);
  }
  else if (new_value < 0)
  {
    if ((ui->spinM->value() > 0) ||
        (ui->spinH->value() > 0) ||
        (ui->spinDays->value() > 0))
    {
      ui->spinM->setValue(ui->spinM->value() - 1);
      ui->spinS->setValue(FACTOR_min + new_value);
    }
    else
      ui->spinS->setValue(0);
  }
  emit valueChanged();
}

void TimeDurationWidget::on_spinM_valueChanged(int new_value)
{
  if (new_value >= FACTOR_h)
  {
    ui->spinH->setValue(ui->spinH->value() + new_value / FACTOR_h);
    if (ui->spinDays->value() != ui->spinDays->maximum())
      ui->spinM->setValue(new_value % FACTOR_h);
    else
      ui->spinM->setValue(0);
  }
  else if (new_value < 0)
  {
    if ((ui->spinH->value() > 0) || (ui->spinDays->value() > 0))
    {
      ui->spinH->setValue(ui->spinH->value() - 1);
      ui->spinM->setValue(FACTOR_h + new_value);
    }
    else
      ui->spinM->setValue(0);
  }
  emit valueChanged();
}

void TimeDurationWidget::on_spinH_valueChanged(int new_value)
{
  if (new_value >= FACTOR_day)
  {
    ui->spinDays->setValue(ui->spinDays->value() + new_value / FACTOR_day);
    if (ui->spinDays->value() != ui->spinDays->maximum())
      ui->spinH->setValue(new_value % FACTOR_day);
    else
      ui->spinH->setValue(0);
  }
  else if (new_value < 0)
  {
    if (ui->spinDays->value() > 0)
    {
      ui->spinDays->setValue(ui->spinDays->value() - 1);
      ui->spinH->setValue(FACTOR_day + new_value);
    }
    else
      ui->spinH->setValue(0);
  }
  emit valueChanged();
}

void TimeDurationWidget::on_spinDays_editingFinished()
{
  emit editingFinished();
}

void TimeDurationWidget::on_spinH_editingFinished()
{
  emit editingFinished();
}

void TimeDurationWidget::on_spinM_editingFinished()
{
  emit editingFinished();
}

void TimeDurationWidget::on_spinS_editingFinished()
{
  emit editingFinished();
}

void TimeDurationWidget::on_spin_ms_editingFinished()
{
  emit editingFinished();
}
