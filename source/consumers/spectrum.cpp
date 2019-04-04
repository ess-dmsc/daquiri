#include <consumers/spectrum.h>

#include <core/util/logger.h>

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

  base_options.branches.add(periodic_trigger_.settings(-1, "Clear periodically"));

  base_options.branches.add(filters_.settings());

  metadata_.overwrite_all_attributes(base_options);
}

void Spectrum::_apply_attributes()
{
  try
  {
    Consumer::_apply_attributes();

    filters_.settings(metadata_.get_attribute("filters"));
    metadata_.replace_attribute(filters_.settings());

    periodic_trigger_.settings(metadata_.get_attribute(periodic_trigger_.settings()));
    metadata_.replace_attribute(periodic_trigger_.settings(-1, "Clear periodically"));
  }
  catch (...)
  {
    std::throw_with_nested(std::runtime_error("<Spectrum> Failed _apply_attributes"));
  }
}

bool Spectrum::_accept_spill(const Spill& spill)
{
  return (Consumer::_accept_spill(spill)
      && (spill.type != Spill::Type::daq_status));
}

void Spectrum::_push_stats_pre(const Spill& spill)
{
  if (!this->_accept_spill(spill))
    return;

  filters_.configure(spill);

  if (metadata_.get_attribute("start_time").time() == hr_time_t())
    metadata_.set_attribute(Setting("start_time", spill.time));

  if (periodic_trigger_.triggered)
  {
    if (data_)
    {
      data_->clear();
      recent_rate_.update(recent_rate_.previous_status, data_->total_count());
    }
    periodic_trigger_.triggered = false;
  }
}

void Spectrum::update_cumulative(const Status& new_status)
{
  if (stats_.size() &&
      (stats_.back().type == Spill::Type::running))
    stats_.pop_back();
  stats_.push_back(new_status);

  auto live_time = Status::total_elapsed(stats_, "live_time");
  auto real_time = Status::total_elapsed(stats_, "native_time");
  if (live_time == hr_duration_t())
    live_time = real_time;
  metadata_.set_attribute(Setting("live_time", live_time));
  metadata_.set_attribute(Setting("real_time", real_time));
}

void Spectrum::_push_stats_post(const Spill& spill)
{
  if (!this->_accept_spill(spill))
    return;

  this->_recalc_axes();

  auto new_status = Status::extract(spill);
  periodic_trigger_.update(new_status);
  update_cumulative(new_status);

  if (data_)
  {
    metadata_.set_attribute(Setting::precise("total_count", data_->total_count()));
    metadata_.set_attribute(recent_rate_.update(new_status, data_->total_count()));
  }
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