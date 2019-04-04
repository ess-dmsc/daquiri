#include <consumers/tof_1d_correlate.h>
#include <consumers/dataspaces/dense1d.h>

#include <core/util/logger.h>

namespace DAQuiri {

TOF1DCorrelate::TOF1DCorrelate()
    : Spectrum()
{
  data_ = std::make_shared<Dense1D>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Time of flight 1D spectrum (with correlation across streams)");

  SettingMeta app("appearance", SettingType::text, "Appearance");
  app.set_flag("color");
  base_options.branches.add(Setting(app));

  SettingMeta res("time_resolution", SettingType::floating, "Time resolution");
  res.set_flag("preset");
  res.set_val("min", 1);
  res.set_val("units", "units (see below)");
  base_options.branches.add(res);

  SettingMeta units("time_units", SettingType::menu, "Time units (domain)");
  units.set_flag("preset");
  units.set_enum(0, "ns");
  units.set_enum(3, "\u03BCs");
  units.set_enum(6, "ms");
  units.set_enum(9, "s");
  base_options.branches.add(units);

  SettingMeta cn("channel_num", SettingType::integer, "Accept events only from channel");
  cn.set_flag("preset");
  cn.set_val("min", 0);
  base_options.branches.add(cn);

  SettingMeta stream("chopper_stream_id", SettingType::text, "Chopper stream ID");
  stream.set_flag("preset");
  stream.set_flag("stream");
  base_options.branches.add(stream);

  metadata_.overwrite_all_attributes(base_options);
}

void TOF1DCorrelate::_apply_attributes()
{
  Spectrum::_apply_attributes();

  //TODO: unused
  channel_num_ = metadata_.get_attribute("channel_num").get_number();

  time_resolution_ = 0;
  if (metadata_.get_attribute("time_resolution").get_number() > 0)
    time_resolution_ = 1.0 / metadata_.get_attribute("time_resolution").get_number();
  auto unit = metadata_.get_attribute("time_units").selection();
  units_name_ = metadata_.get_attribute("time_units").metadata().enum_name(unit);
  units_multiplier_ = std::pow(10, unit);
  time_resolution_ /= units_multiplier_;

  chopper_stream_id_ = metadata_.get_attribute("chopper_stream_id").get_text();

  this->_recalc_axes();
}

void TOF1DCorrelate::_init_from_file()
{
  domain_ = data_->axis(0).domain;
  Spectrum::_init_from_file();
}

void TOF1DCorrelate::_recalc_axes()
{
  CalibID id("time", "", units_name_);
  data_->set_axis(0, DataAxis(Calibration(id, id), domain_));
}

bool TOF1DCorrelate::_accept_spill(const Spill& spill)
{
  return (spill.stream_id == chopper_stream_id_) || Spectrum::_accept_spill(spill);
}

bool TOF1DCorrelate::_accept_events(const Spill& spill)
{
//  return ((pulse_time_ >= 0) && (0 != time_resolution_));
  return (Spectrum::_accept_spill(spill) && (0 != time_resolution_));
}

void TOF1DCorrelate::_push_stats_pre(const Spill& spill)
{
  if (!this->_accept_spill(spill))
    return;

  if (spill.stream_id == chopper_stream_id_)
  {
    chopper_timebase_ = spill.event_model.timebase;
    for (auto& e : spill.events)
    {
      //if channel matches?
      chopper_buffer_.push_back(e.timestamp());
    }
  }
  else
  {
    timebase_ = spill.event_model.timebase;
  }

  Spectrum::_push_stats_pre(spill);
}

void TOF1DCorrelate::_push_stats_post(const Spill& spill)
{
  if (!this->_accept_spill(spill))
    return;

  while (can_bin() && bin_events())
  {
//    DBG( "Binning " << events_buffer_.size() << " in " << chopper_buffer_.size();
  }

  Spectrum::_push_stats_post(spill);
}

bool TOF1DCorrelate::can_bin() const
{
  if ((chopper_buffer_.size() < 2) || events_buffer_.empty())
    return false;

  uint64_t second_pulse = chopper_buffer_.at(1);

  return (events_buffer_.back() > second_pulse);
}

bool TOF1DCorrelate::bin_events()
{
  PreciseFloat offset = chopper_timebase_.to_nanosec(chopper_buffer_.front());
  chopper_buffer_.pop_front();
  PreciseFloat limit = chopper_timebase_.to_nanosec(chopper_buffer_.front());

  while (!events_buffer_.empty())
  {
    PreciseFloat t = timebase_.to_nanosec(events_buffer_.front());
    if (t >= limit)
      return false;

    events_buffer_.pop_front();
    if (t < offset)
      continue;

    double nsecs = t - offset;

    if (nsecs < 0.0)
      continue;

    coords_[0] = static_cast<size_t>(nsecs * time_resolution_);

    if (coords_[0] >= domain_.size())
    {
      size_t oldbound = domain_.size();
      domain_.resize(coords_[0] + 1);

      for (size_t i = oldbound; i <= coords_[0]; ++i)
        domain_[i] = i / time_resolution_ / units_multiplier_;
    }

    data_->add_one(coords_);
  }

  return true;
}

void TOF1DCorrelate::_push_event(const Event& event)
{
//  if ((event.value_count() < 2) || (event.value(1) != channel_num_))
//    return;
//  if (Spectrum::_accept_spill(spill) && (event.value(1) != channel_num_))
//    return;

  if (!filters_.accept(event))
    return;

  events_buffer_.push_back(event.timestamp());
}

}
