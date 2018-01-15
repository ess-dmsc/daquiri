#include "tof_1d_correlate.h"
#include "dense1d.h"

#include "custom_logger.h"

#define kDimensions 1

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

  SettingMeta stream("pulse_stream_id", SettingType::text, "Pulse stream ID");
  stream.set_flag("preset");
  base_options.branches.add(stream);

  metadata_.overwrite_all_attributes(base_options);
}

bool TOF1DCorrelate::_initialize()
{
  Spectrum::_initialize();

  channel_num_ = metadata_.get_attribute("channel_num").get_number();

  time_resolution_ = 1.0 / metadata_.get_attribute("time_resolution").get_number();
  auto unit = metadata_.get_attribute("time_units").selection();
  units_name_ = metadata_.get_attribute("time_units").metadata().enum_name(unit);
  units_multiplier_ = std::pow(10, unit);
  time_resolution_ /= units_multiplier_;

  pulse_stream_id_ = metadata_.get_attribute("pulse_stream_id").get_text();

  this->_recalc_axes();
  return true;
}

void TOF1DCorrelate::_init_from_file()
{
//  metadata_.set_attribute(Setting::integer("time_resolution", bits_));

  Spectrum::_init_from_file();
}

void TOF1DCorrelate::_set_detectors(const std::vector<Detector>& dets)
{
  metadata_.detectors.resize(kDimensions, Detector());

  if (dets.size() == kDimensions)
    metadata_.detectors = dets;

  if (dets.size() >= kDimensions)
  {
    for (size_t i=0; i < dets.size(); ++i)
    {
      if (metadata_.chan_relevant(i))
      {
        metadata_.detectors[0] = dets[i];
        break;
      }
    }
  }

  this->_recalc_axes();
}

void TOF1DCorrelate::_recalc_axes()
{
  CalibID id("time", "", units_name_);
  data_->set_axis(0, DataAxis(Calibration(id, id), domain_));
}

bool TOF1DCorrelate::_accept_spill(const Spill& spill)
{
  return (spill.stream_id == pulse_stream_id_) || Spectrum::_accept_spill(spill);
}

bool TOF1DCorrelate::_accept_events()
{
//  return ((pulse_time_ >= 0) && (0 != time_resolution_));
  return (0 != time_resolution_);
}

void TOF1DCorrelate::_push_stats_pre(const Spill& spill)
{
  if (!this->_accept_spill(spill))
    return;

  if (spill.stream_id == pulse_stream_id_)
  {
    pulse_timebase_ = spill.event_model.timebase;

    if (spill.type == StatusType::running)
    {
      for (auto &e : spill.events)
      {
        //if channel matches?
        pulse_buffer_.push_back(e.timestamp());
      }

      while (can_bin())
        bin_events();
    }
  }
  else
  {
    timebase_ = spill.event_model.timebase;
  }


  Spectrum::_push_stats_pre(spill);
}

bool TOF1DCorrelate::can_bin() const
{

  if ((pulse_buffer_.size() < 2) || events_buffer_.empty())
    return false;

  uint64_t second_pulse = pulse_buffer_.at(1);

  return (events_buffer_.back() > second_pulse);
}

void TOF1DCorrelate::bin_events()
{
  uint64_t offset = pulse_buffer_.front();
  pulse_buffer_.pop_front();

  uint64_t limit = pulse_buffer_.front();

  while (!events_buffer_.empty())
  {
    uint64_t t = events_buffer_.front();
    if (t >= limit)
      break;

    events_buffer_.pop_front();
    if (t < offset)
      continue;

    double nsecs = timebase_.to_nanosec(t) - pulse_timebase_.to_nanosec(offset);

    if (nsecs < 0)
      return;

    coords_[0] = static_cast<size_t>(nsecs * time_resolution_);

    if (coords_[0] >= domain_.size())
    {
      size_t oldbound = domain_.size();
      domain_.resize(coords_[0]+1);

      for (size_t i=oldbound; i <= coords_[0]; ++i)
        domain_[i] = i / time_resolution_ / units_multiplier_;
    }

    data_->add_one(coords_);
    total_count_++;
    recent_count_++;
  }
}

void TOF1DCorrelate::_push_event(const Event& event)
{
  if ((event.value_count() < 2) || (event.value(1) != channel_num_))
    return;
//  if (Spectrum::_accept_spill(spill) && (event.value(1) != channel_num_))
//    return;

  events_buffer_.push_back(event.timestamp());

}