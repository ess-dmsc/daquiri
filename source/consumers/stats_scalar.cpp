#include "stats_scalar.h"
#include "scalar.h"

#include "custom_logger.h"

StatsScalar::StatsScalar()
    : Spectrum()
{
  data_ = std::make_shared<Scalar>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Scalar stats value");

  SettingMeta app("appearance", SettingType::text, "Appearance");
  app.set_flag("color");
  base_options.branches.add(Setting(app));

  SettingMeta swhat("what_stats", SettingType::menu, "What stats?");
  swhat.set_enum(0, "events per spill");
  swhat.set_enum(1, "% dead time");
  swhat.set_enum(2, "engine queue size");
  swhat.set_enum(3, "engine dropped spills");
  swhat.set_enum(4, "engine dropped events");
  base_options.branches.add(swhat);

  metadata_.overwrite_all_attributes(base_options);
}

void StatsScalar::_apply_attributes()
{
  Spectrum::_apply_attributes();
  what_ = metadata_.get_attribute("what_stats").selection();
  this->_recalc_axes();
}

void StatsScalar::_recalc_axes()
{
  data_->recalc_axes();
}

bool StatsScalar::_accept_spill(const Spill& spill)
{
  return (Spectrum::_accept_spill(spill));
}

bool StatsScalar::_accept_events(const Spill& /*spill*/)
{
  return false;
}

void StatsScalar::_push_stats_pre(const Spill& spill)
{
  if (!this->_accept_spill(spill))
    return;

  if (what_ == 0)
  {
    entry_.second = spill.events.size();
    recent_count_++;
    data_->add(entry_);
  }
  else if (what_ == 2)
  {
    entry_.second =  spill.state.find(Setting("daquiri_queue_size")).get_int();
    recent_count_++;
    data_->add(entry_);
  }
  else if (what_ == 3)
  {
    entry_.second = spill.state.find(Setting("daquiri_queue_dropped_spills")).get_int();
    recent_count_++;
    data_->add(entry_);
  }
  else if (what_ == 4)
  {
    entry_.second = spill.state.find(Setting("daquiri_queue_dropped_events")).get_int();
    recent_count_++;
    data_->add(entry_);
  }
  Spectrum::_push_stats_pre(spill);
}

void StatsScalar::_push_stats_post(const Spill& spill)
{
  if (!this->_accept_spill(spill))
    return;

  Spectrum::_push_stats_post(spill);

  if (what_ == 1)
  {
    double real = metadata_.get_attribute("real_time").duration().total_milliseconds();
    if (real > 0)
    {
      double live = metadata_.get_attribute("live_time").duration().total_milliseconds();
      entry_.second = (real - live) / real * 100.0;
      recent_count_++;
      data_->add(entry_);
    }
  }
}

void StatsScalar::_push_event(const Event& event)
{
}
