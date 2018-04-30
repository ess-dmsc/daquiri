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

  SettingMeta stat("what_stats", SettingType::text, "Stat of choice");
  stat.set_flag("stat_value");
  base_options.branches.add(Setting(stat));


  metadata_.overwrite_all_attributes(base_options);
}

void StatsScalar::_apply_attributes()
{
  Spectrum::_apply_attributes();
  what_ = metadata_.get_attribute("what_stats").get_text();
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

  auto set = spill.state.find(Setting(what_));
  if (set)
  {
    entry_.second = set.get_number();
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

//  if (what_ == 1)
//  {
//    double real = metadata_.get_attribute("real_time").duration().total_milliseconds();
//    if (real > 0)
//    {
//      double live = metadata_.get_attribute("live_time").duration().total_milliseconds();
//      entry_.second = (real - live) / real * 100.0;
//      recent_count_++;
//      data_->add(entry_);
//    }
//  }
}

void StatsScalar::_push_event(const Event& event)
{
}
