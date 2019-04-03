#include "TimeDurationWidget.h"
#include "ui_TimeDurationWidget.h"
#include <date/date.h>

#include <core/util/logger.h>

static constexpr uint64_t kFactorMetricOrder {1000};
static constexpr uint64_t kFactorMinutesHours {60};
static constexpr uint64_t kFactorDay {24};

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
  ui->label_ms->setVisible(use);
  ui->label_us->setVisible(use);
  ui->spin_ms->setVisible(use);
  ui->spin_us->setVisible(use);
}

uint64_t TimeDurationWidget::total_seconds()
{
  uint64_t time = kFactorDay * ui->spinDays->value();
  time += ui->spinH->value();
  time *= kFactorMinutesHours;
  time += ui->spinM->value();
  time *= kFactorMinutesHours;
  time += ui->spinS->value();
  return time;
}

void TimeDurationWidget::set_total_seconds(uint64_t secs)
{
  ui->spin_us->setValue(0);
  ui->spin_ms->setValue(0);
  ui->spinS->setValue(secs % kFactorMinutesHours);
  uint64_t whole_minutes = secs / kFactorMinutesHours;
  ui->spinM->setValue(whole_minutes % kFactorMinutesHours);
  uint64_t whole_hours = whole_minutes / kFactorMinutesHours;
  ui->spinH->setValue(whole_hours % kFactorDay);
  ui->spinDays->setValue(whole_hours / kFactorDay);
}

void TimeDurationWidget::set_duration(hr_duration_t duration)
{
  using namespace date;
  uint64_t total_us = static_cast<uint64_t>(round<std::chrono::microseconds>(duration).count());
  ui->spin_us->setValue(total_us % kFactorMetricOrder);
  uint64_t whole_milliseconds = total_us / kFactorMetricOrder;
  ui->spin_ms->setValue(whole_milliseconds % kFactorMetricOrder);
  uint64_t whole_seconds = whole_milliseconds / kFactorMetricOrder;
  ui->spinS->setValue(whole_seconds % kFactorMinutesHours);
  uint64_t whole_minutes = whole_seconds / kFactorMinutesHours;
  ui->spinM->setValue(whole_minutes % kFactorMinutesHours);
  uint64_t whole_hours = whole_minutes / kFactorMinutesHours;
  ui->spinH->setValue(whole_hours % kFactorDay);
  ui->spinDays->setValue(whole_hours / kFactorDay);
}

hr_duration_t TimeDurationWidget::get_duration() const
{
  uint64_t time = kFactorDay * ui->spinDays->value();
  time += ui->spinH->value();
  time *= kFactorMinutesHours;
  time += ui->spinM->value();
  time *= kFactorMinutesHours;
  time += ui->spinS->value();
  time *= kFactorMetricOrder;
  time += ui->spin_ms->value();
  time *= kFactorMetricOrder;
  time += ui->spin_us->value();
  hr_duration_t ret = std::chrono::microseconds(time);
  return ret;
}

void TimeDurationWidget::on_spin_us_valueChanged(int val)
{
  if (val >= static_cast<int>(kFactorMetricOrder))
  {
    auto new_value = static_cast<uint64_t>(val);
    ui->spin_ms->setValue(ui->spin_ms->value() + new_value / kFactorMetricOrder);
    if (ui->spinDays->value() != ui->spinDays->maximum())
      ui->spin_us->setValue(new_value % kFactorMetricOrder);
    else
      ui->spin_us->setValue(0);
  }
  else if (val < 0)
  {
    if ((ui->spin_ms->value() > 0) ||
        (ui->spinS->value() > 0) ||
        (ui->spinM->value() > 0) ||
        (ui->spinH->value() > 0)  ||
        (ui->spinDays->value() > 0))
    {
      ui->spin_ms->setValue(ui->spin_ms->value() - 1);
      ui->spin_us->setValue(kFactorMetricOrder + val);
    }
    else
    {
      ui->spin_us->setValue(0);
    }
  }
  emit valueChanged();
}

void TimeDurationWidget::on_spin_ms_valueChanged(int val)
{
  if (val >= static_cast<int>(kFactorMetricOrder))
  {
    auto new_value = static_cast<uint64_t>(val);
    ui->spinS->setValue(ui->spinS->value() + new_value / kFactorMetricOrder);
    if (ui->spinDays->value() != ui->spinDays->maximum())
      ui->spin_ms->setValue(new_value % kFactorMetricOrder);
    else
      ui->spin_ms->setValue(0);
  }
  else if (val < 0)
  {
    if ((ui->spinS->value() > 0) ||
        (ui->spinM->value() > 0) ||
        (ui->spinH->value() > 0)  ||
        (ui->spinDays->value() > 0))
    {
      ui->spinS->setValue(ui->spinS->value() - 1);
      ui->spin_ms->setValue(kFactorMetricOrder + val);
    }
    else
    {
      ui->spin_ms->setValue(0);
    }
  }
  emit valueChanged();
}


void TimeDurationWidget::on_spinS_valueChanged(int val)
{
  if (val >= static_cast<int>(kFactorMinutesHours))
  {
    auto new_value = static_cast<uint64_t>(val);
    ui->spinM->setValue(ui->spinM->value() + new_value / kFactorMinutesHours);
    if (ui->spinDays->value() != ui->spinDays->maximum())
      ui->spinS->setValue(new_value % kFactorMinutesHours);
    else
      ui->spinS->setValue(0);
  }
  else if (val < 0)
  {
    if ((ui->spinM->value() > 0) ||
        (ui->spinH->value() > 0) ||
        (ui->spinDays->value() > 0))
    {
      ui->spinM->setValue(ui->spinM->value() - 1);
      ui->spinS->setValue(kFactorMinutesHours + val);
    }
    else
      ui->spinS->setValue(0);
  }
  emit valueChanged();
}

void TimeDurationWidget::on_spinM_valueChanged(int val)
{
  if (val >= static_cast<int>(kFactorMinutesHours))
  {
    auto new_value = static_cast<uint64_t>(val);
    ui->spinH->setValue(ui->spinH->value() + new_value / kFactorMinutesHours);
    if (ui->spinDays->value() != ui->spinDays->maximum())
      ui->spinM->setValue(new_value % kFactorMinutesHours);
    else
      ui->spinM->setValue(0);
  }
  else if (val < 0)
  {
    if ((ui->spinH->value() > 0) || (ui->spinDays->value() > 0))
    {
      ui->spinH->setValue(ui->spinH->value() - 1);
      ui->spinM->setValue(kFactorMinutesHours + val);
    }
    else
      ui->spinM->setValue(0);
  }
  emit valueChanged();
}

void TimeDurationWidget::on_spinH_valueChanged(int val)
{
  if (val >= static_cast<int>(kFactorDay))
  {
    auto new_value = static_cast<uint64_t>(val);
    ui->spinDays->setValue(ui->spinDays->value() + new_value / kFactorDay);
    if (ui->spinDays->value() != ui->spinDays->maximum())
      ui->spinH->setValue(new_value % kFactorDay);
    else
      ui->spinH->setValue(0);
  }
  else if (val < 0)
  {
    if (ui->spinDays->value() > 0)
    {
      ui->spinDays->setValue(ui->spinDays->value() - 1);
      ui->spinH->setValue(kFactorDay + val);
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

void TimeDurationWidget::on_spin_us_editingFinished()
{
  emit editingFinished();
}
