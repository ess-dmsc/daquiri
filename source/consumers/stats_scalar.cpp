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

  metadata_.overwrite_all_attributes(base_options);
}

void StatsScalar::_apply_attributes()
{
  Spectrum::_apply_attributes();
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
  if (this->_accept_spill(spill))
  {
    entry_.second = spill.events.size();
    data_->add(entry_);
    Spectrum::_push_stats_pre(spill);
  }
}

void StatsScalar::_push_event(const Event& event)
{
  recent_count_++;
}
