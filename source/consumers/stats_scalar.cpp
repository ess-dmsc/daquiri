#include <consumers/stats_scalar.h>
#include <consumers/dataspaces/scalar.h>

#include <core/util/logger.h>

namespace DAQuiri {

StatsScalar::StatsScalar()
    : Spectrum()
{
  data_ = std::make_shared<Scalar>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Scalar stats value");

  SettingMeta app("appearance", SettingType::text, "Appearance");
  app.set_flag("gradient-name");
  base_options.branches.add(app);

  SettingMeta stat("what_stats", SettingType::text, "Stat of choice");
  stat.set_flag("stat_value");
  base_options.branches.add(Setting(stat));

  SettingMeta diff("diff", SettingType::boolean, "Diff cumulative value");
  base_options.branches.add(Setting(diff));

  // TODO: Unused
  SettingMeta eul("enforce_upper_limit", SettingType::boolean, "Enforce upper limit");
  base_options.branches.add(Setting(eul));

  // TODO: Unused
  SettingMeta ul("upper_limit", SettingType::floating, "Enforce upper limit");
  base_options.branches.add(Setting(ul));

  metadata_.overwrite_all_attributes(base_options);
}

void StatsScalar::_apply_attributes()
{
  Spectrum::_apply_attributes();
  what_ = metadata_.get_attribute("what_stats").get_text();
  diff_ = metadata_.get_attribute("diff").get_bool();
  this->_recalc_axes();
}

void StatsScalar::_recalc_axes()
{
  data_->recalc_axes();
}

void StatsScalar::_push_stats_pre(const Spill& spill)
{
  if (!this->_accept_spill(spill))
    return;

  auto set = spill.state.find(Setting(what_));
  if (set)
  {
    entry_.second = set.get_number();
    if (diff_)
    {
      entry_.second -= previous_;
      previous_ = set.get_number();
    }
    data_->add(entry_);
  }

  Spectrum::_push_stats_pre(spill);
}

//void StatsScalar::_push_stats_post(const Spill& spill)
//{
//  if (!this->_accept_spill(spill))
//    return;
//
//  Spectrum::_push_stats_post(spill);
//
////  if (what_ == 1)
////  {
////    double real = metadata_.get_attribute("real_time").duration().total_milliseconds();
////    if (real > 0)
////    {
////      double live = metadata_.get_attribute("live_time").duration().total_milliseconds();
////      entry_.second = (real - live) / real * 100.0;
////      data_->add(entry_);
////    }
////  }
//}

bool StatsScalar::_accept_events(const Spill& /*spill*/)
{
  return false;
}

void StatsScalar::_push_event(const Event& /*event*/)
{
  // do nothing here
  // this should never be called anyhow because of above
}

void StatsScalar::_flush()
{
  if (diff_)
  {
    entry_.second = 0;
    data_->add(entry_);
  }

  Spectrum::_flush();
}


}
