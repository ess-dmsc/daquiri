#include "spectrum.h"

#include "custom_logger.h"

namespace DAQuiri {

Spectrum::Spectrum()
    : Consumer()
{
  Setting base_options = metadata_.attributes();

  SettingMeta name("name", SettingType::text, "Name");
  base_options.branches.add(name);

  SettingMeta vis("visible", SettingType::boolean, "Visible");
  base_options.branches.add(vis);

  SettingMeta sca("preferred_scale", SettingType::menu, "Plot scale for counts");
  sca.set_enum(0, "Linear");
  sca.set_enum(1, "Logarithmic");
  base_options.branches.add(sca);

  SettingMeta totalcount("total_count", SettingType::precise, "Total count");
  totalcount.set_flag("readonly");
  totalcount.set_val("min", 0);
  base_options.branches.add(totalcount);

  SettingMeta start_time("start_time", SettingType::time, "Start time");
  start_time.set_flag("readonly");
  base_options.branches.add(start_time);

  SettingMeta live_time("live_time", SettingType::duration, "Live time");
  live_time.set_flag("readonly");
  base_options.branches.add(live_time);

  SettingMeta real_time("real_time", SettingType::duration, "Real time");
  real_time.set_flag("readonly");
  base_options.branches.add(real_time);

  base_options.branches.add(recent_rate_.update(Status(), 0));

  base_options.branches.add(periodic_trigger_.settings(0));

  metadata_.overwrite_all_attributes(base_options);
}

void Spectrum::_apply_attributes()
{
  Consumer::_apply_attributes();

  periodic_trigger_.settings(metadata_.get_attribute(periodic_trigger_.settings(0)));
  metadata_.replace_attribute(periodic_trigger_.settings(0));
}

bool Spectrum::_accept_spill(const Spill& spill)
{
  return (Consumer::_accept_spill(spill)
      && (spill.type != StatusType::daq_status));
}

void Spectrum::_push_stats_pre(const Spill& spill)
{
  if (!this->_accept_spill(spill))
    return;

  if (metadata_.get_attribute("start_time").time().is_not_a_date_time())
    metadata_.set_attribute(Setting("start_time", spill.time));

  if (periodic_trigger_.triggered_)
  {
    if (data_)
    {
      data_->clear();
      recent_rate_.update(recent_rate_.previous_status, data_->total_count());
    }
    periodic_trigger_.triggered_ = false;
  }
}

void Spectrum::update_cumulative(const Status& new_status)
{
  if (stats_.size() &&
      (stats_.back().type == StatusType::running))
    stats_.pop_back();
  stats_.push_back(new_status);

  auto live_time = Status::total_elapsed(stats_, "live_time");
  auto real_time = Status::total_elapsed(stats_, "native_time");
  if (live_time.is_not_a_date_time())
    live_time = real_time;
  metadata_.set_attribute(Setting("live_time", live_time));
  metadata_.set_attribute(Setting("real_time", real_time));
}

void Spectrum::_push_stats_post(const Spill& spill)
{
  if (!this->_accept_spill(spill))
    return;

  auto new_status = Status::extract(spill);

  periodic_trigger_.update(new_status);

  update_cumulative(new_status);

  if (data_)
  {
    metadata_.set_attribute(Setting::precise("total_count", data_->total_count()));
    metadata_.set_attribute(recent_rate_.update(new_status, data_->total_count()));
  }

  this->_recalc_axes();
}

void Spectrum::_flush()
{
  if (!data_)
    return;
  metadata_.set_attribute(Setting::precise("total_count", data_->total_count()));
  metadata_.set_attribute(
      recent_rate_.update(recent_rate_.previous_status, data_->total_count()));
}

}