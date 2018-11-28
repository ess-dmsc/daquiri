#include <consumers/prebinned_1d.h>
#include <consumers/dataspaces/dense1d.h>

#include <core/util/custom_logger.h>

namespace DAQuiri {

Prebinned1D::Prebinned1D()
    : Spectrum()
{
  data_ = std::make_shared<Dense1D>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Prebinned 1D spectrum (or trace)");

  SettingMeta app("appearance", SettingType::text, "Appearance");
  app.set_flag("color");
  base_options.branches.add(app);

  SettingMeta val_name("trace_id", SettingType::text);
  val_name.set_flag("preset");
  val_name.set_flag("event_trace");
  val_name.set_val("description", "Name of event trace to bin");
  base_options.branches.add(val_name);

  SettingMeta mds("downsample", SettingType::integer, "Downsample by");
  mds.set_val("units", "bits");
  mds.set_flag("preset");
  mds.set_val("min", 0);
  mds.set_val("max", 31);
  base_options.branches.add(mds);

  metadata_.overwrite_all_attributes(base_options);
}

void Prebinned1D::_apply_attributes()
{
  Spectrum::_apply_attributes();
  trace_name_ = metadata_.get_attribute("trace_id").get_text();
  downsample_ = static_cast<uint16_t>(metadata_.get_attribute("downsample").get_int());
  this->_recalc_axes();
}

void Prebinned1D::_recalc_axes()
{
  Detector det;
  if (data_->dimensions() == metadata_.detectors.size())
    det = metadata_.detectors[0];

  auto calib = det.get_calibration({trace_name_, det.id()}, {trace_name_});
  data_->set_axis(0, DataAxis(calib, downsample_));

  data_->recalc_axes();
}

// TODO: check trace dimensions

bool Prebinned1D::_accept_spill(const Spill& spill)
{
  return (Spectrum::_accept_spill(spill)
      &&
          spill.event_model.name_to_trace.count(trace_name_));
}

bool Prebinned1D::_accept_events(const Spill& /*spill*/)
{
  return (trace_idx_ >= 0);
}

void Prebinned1D::_push_stats_pre(const Spill& spill)
{
  if (this->_accept_spill(spill))
  {
    trace_idx_ = spill.event_model.name_to_trace.at(trace_name_);
    Spectrum::_push_stats_pre(spill);
  }
}

void Prebinned1D::_push_event(const Event& event)
{
  if (!filters_.accept(event))
    return;

  const auto& trace = event.trace(trace_idx_);

  for (size_t i = 0; i < trace.size(); ++i)
  {
    entry_.first[0] = (i >> downsample_);
    entry_.second = trace[i];
    data_->add(entry_);
  }
}

}
