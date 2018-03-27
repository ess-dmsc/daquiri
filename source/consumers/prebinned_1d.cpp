#include "prebinned_1d.h"
#include "dense1d.h"

#include "custom_logger.h"

Prebinned1D::Prebinned1D()
  : Spectrum()
{
  data_ = std::make_shared<Dense1D>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Prebinned 1D spectrum (or trace)");

  SettingMeta app("appearance", SettingType::text, "Appearance");
  app.set_flag("color");
  base_options.branches.add(Setting(app));

  SettingMeta val_name("value_name", SettingType::text);
  val_name.set_flag("preset");
  val_name.set_flag("event_trace");
  val_name.set_val("description", "Name of event value to bin");
  base_options.branches.add(val_name);

  metadata_.overwrite_all_attributes(base_options);
}

void Prebinned1D::_apply_attributes()
{
  Spectrum::_apply_attributes();
  trace_name_ = metadata_.get_attribute("value_name").get_text();
  this->_recalc_axes();
}

void Prebinned1D::_init_from_file()
{
  domain_ = data_->axis(0).domain;
  Spectrum::_init_from_file();
}

void Prebinned1D::_recalc_axes()
{
  CalibID id(trace_name_);
  DataAxis ax;
  ax.calibration = Calibration(id, id);
  ax.domain = domain_;
  data_->set_axis(0, ax);
}

bool Prebinned1D::_accept_spill(const Spill& spill)
{
  return (Spectrum::_accept_spill(spill)
          &&
          spill.event_model.name_to_trace.count(trace_name_));
}

bool Prebinned1D::_accept_events(const Spill &spill)
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
  const auto& trace = event.trace(trace_idx_);

  if (trace.size() >= domain_.size())
  {
    size_t oldbound = domain_.size();
    domain_.resize(trace.size());

    for (size_t i=oldbound; i < trace.size(); ++i)
      domain_[i] = i;
  }

  for (size_t i=0; i < trace.size(); ++i)
  {
    entry_.first[0] = i;
    entry_.second = trace[i];
    data_->add(entry_);
  }

  total_count_++;
  recent_count_++;
}
