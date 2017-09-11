#include "prebinned_1d.h"
#include "dense1d.h"

#include "custom_logger.h"

#define kDimensions 1

Prebinned1D::Prebinned1D()
  : Spectrum()
{
  data_ = std::make_shared<Dense1D>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Prebinned 1D spectrum (or trace)");

  SettingMeta app("appearance", SettingType::color);
  app.set_val("description", "Plot appearance");
  base_options.branches.add(Setting(app));

  SettingMeta val_name("value_name", SettingType::text);
  val_name.set_flag("preset");
  val_name.set_val("description", "Name of event value to bin");
  base_options.branches.add(val_name);

  SettingMeta add_channels("add_channels", SettingType::pattern, "Channels to bin");
  add_channels.set_flag("preset");
  add_channels.set_val("chans", 1);
  base_options.branches.add(add_channels);

  metadata_.overwrite_all_attributes(base_options);
}

bool Prebinned1D::_initialize()
{
  Spectrum::_initialize();
  trace_name_ = metadata_.get_attribute("value_name").get_text();
  channels_ = metadata_.get_attribute("add_channels").pattern();

  this->_recalc_axes();
  return true;
}

void Prebinned1D::_init_from_file()
{
  channels_.resize(1);
  channels_.set_gates(std::vector<bool>({true}));

  metadata_.set_attribute(Setting("add_channels", channels_));

  Spectrum::_init_from_file();
}

void Prebinned1D::_set_detectors(const std::vector<Detector>& dets)
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

void Prebinned1D::_recalc_axes()
{
  CalibID id("", trace_name_, "", 0);
  DataAxis ax;
  ax.calibration = Calibration(id, id);
  ax.domain = domain_;
  data_->set_axis(0, ax);
}

bool Prebinned1D::channel_relevant(int16_t channel) const
{
  return ((channel >= 0) && channels_.relevant(channel));
}

void Prebinned1D::_push_stats(const Status& newBlock)
{
  if (!this->channel_relevant(newBlock.channel()))
    return;

  Spectrum::_push_stats(newBlock);

  if (newBlock.channel() >= static_cast<int16_t>(trace_idx_.size()))
    trace_idx_.resize(newBlock.channel() + 1, -1);
  if (newBlock.event_model().name_to_trace.count(trace_name_))
    trace_idx_[newBlock.channel()] = newBlock.event_model().name_to_trace.at(trace_name_);
}

void Prebinned1D::_push_event(const Event& e)
{
  const auto& c = e.channel();

  if (!this->channel_relevant(c) ||
      !value_relevant(c, trace_idx_))
    return;

  const auto trace = e.trace(trace_idx_.at(c));

  if (trace.size() >= domain_.size())
  {
    size_t oldbound = domain_.size();
    domain_.resize(trace.size());

    for (size_t i=oldbound; i < trace.size(); ++i)
      domain_[i] = i;
  }

  for (size_t i=0; i < trace.size(); ++i)
    data_->add({{i}, trace[i]});

  total_count_++;
  recent_count_++;
}
