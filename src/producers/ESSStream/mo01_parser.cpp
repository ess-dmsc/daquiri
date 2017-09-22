#include "mo01_parser.h"
#include "mo01_nmx_generated.h"

#include "custom_timer.h"
#include "custom_logger.h"

mo01_nmx::mo01_nmx()
  : fb_parser()
{
  std::string mp {"mo01_nmx/"};

  SettingMeta hc(mp + "ChannelHists", SettingType::integer, "Output channel for histograms");
  hc.set_val("min", 0);
  add_definition(hc);

  SettingMeta ctx(mp + "ChannelTraceX", SettingType::integer, "Output channel for trace X");
  ctx.set_val("min", 0);
  add_definition(ctx);

  SettingMeta cty(mp + "ChannelTraceY", SettingType::integer, "Output channel for trace Y");
  cty.set_val("min", 0);
  add_definition(cty);

  SettingMeta root("mo01_nmx", SettingType::stem);
  root.set_flag("producer");
  root.set_enum(0, mp + "ChannelHists");
  root.set_enum(1, mp + "ChannelTraceX");
  root.set_enum(2, mp + "ChannelTraceY");
  add_definition(root);

  hists_model_.add_trace("strips_x", {UINT16_MAX+1});
  hists_model_.add_trace("strips_y", {UINT16_MAX+1});
  hists_model_.add_trace("adc_x", {UINT16_MAX+1});
  hists_model_.add_trace("adc_y", {UINT16_MAX+1});
  hists_model_.add_trace("adc_cluster", {UINT16_MAX+1});

  trace_model_.add_value("strip", 0);
  trace_model_.add_value("time", 0);
  trace_model_.add_value("adc", 0);

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

void mo01_nmx::read_settings_bulk(Setting &set) const
{
  set = enrich_and_toggle_presets(set);
  set.set(Setting::integer("mo01_nmx/ChannelHists", hists_channel_));
  set.set(Setting::integer("mo01_nmx/ChannelTraceX", trace_x_channel_));
  set.set(Setting::integer("mo01_nmx/ChannelTraceY", trace_y_channel_));
}

void mo01_nmx::write_settings_bulk(const Setting& settings)
{
  auto set = enrich_and_toggle_presets(settings);
  hists_channel_ = set.find({"mo01_nmx/ChannelHists"}).get_number();
  trace_x_channel_ = set.find({"mo01_nmx/ChannelTraceX"}).get_number();
  trace_y_channel_ = set.find({"mo01_nmx/ChannelTraceY"}).get_number();
}

SpillPtr mo01_nmx::start_spill() const
{
  return Spill::make_new(StatusType::start,
  {hists_channel_, trace_x_channel_, trace_y_channel_});
}

SpillPtr mo01_nmx::stop_spill() const
{
  return Spill::make_new(StatusType::stop,
  {hists_channel_, trace_x_channel_, trace_y_channel_});
}

SpillPtr mo01_nmx::process_payload(void* msg, TimeBase tb,
                                 uint64_t utime,
                                 PayloadStats &stats)
{
  SpillPtr ret;
  auto em = GetMonitorMessage(msg);
  if (is_empty(*em))
    return nullptr;

  CustomTimer timer(true);

  auto type = em->data_type();
  if (type == DataField::GEMHist)
  {
    hists_model_.timebase = tb;
    ret = Spill::make_new(StatusType::running, {hists_channel_});
    ret->stats[hists_channel_].set_model(hists_model_);

    produce_hists(*em->data_as_GEMHist(), utime, ret);
  }

  if (type == DataField::GEMTrack)
  {
    trace_model_.timebase = tb;
    ret = Spill::make_new(StatusType::running,
    {trace_x_channel_, trace_y_channel_});
    ret->stats[trace_x_channel_].set_model(trace_model_);
    ret->stats[trace_y_channel_].set_model(trace_model_);

    produce_tracks(*em->data_as_GEMTrack(), utime, ret);
  }

  stats.time_start = stats.time_end = utime;

  //  ret->stats[traces_channel_].set_value("pulse_time", time_high);
  stats.time_spent += timer.s();

  return ret;
}

bool mo01_nmx::is_empty(const MonitorMessage& m)
{
  auto type = m.data_type();
  if (type == DataField::GEMHist)
  {
    auto hist = m.data_as_GEMHist();
    if (hist->xstrips()->Length())
      return false;
    if (hist->ystrips()->Length())
      return false;
  }
  if (type == DataField::GEMTrack)
  {
    auto track = m.data_as_GEMTrack();
    if (track->xtrack()->Length())
      return false;
    if (track->ytrack()->Length())
      return false;
  }
  return true;
}

void mo01_nmx::produce_hists(const GEMHist& hist, uint64_t utime, SpillPtr ret)
{
//  DBG << "Received GEMHist\n" << debug(hist);

  Event e(hists_channel_, hists_model_);
  e.set_native_time(utime);

  grab_hist(e, 0, hist.xstrips());
  grab_hist(e, 1, hist.ystrips());
  grab_hist(e, 2, hist.xspectrum());
  grab_hist(e, 3, hist.yspectrum());
  grab_hist(e, 4, hist.cluster_spectrum());

  ret->events.push_back(e);
}

void mo01_nmx::grab_hist(Event& e, size_t idx, const flatbuffers::Vector<uint32_t>* data)
{
  if (!data->Length())
    return;
  std::vector<uint32_t> vals(data->Length(), 0);
  for (size_t i=0; i < data->Length(); ++i)
    vals[i] = data->Get(i);
  e.set_trace(idx, vals);
}

void mo01_nmx::produce_tracks(const GEMTrack& track, uint64_t utime, SpillPtr ret)
{
//  DBG << "Received GEMTrack\n" << debug(track);

//  auto time = track.time_offset();
  grab_track(track.xtrack(), utime, trace_x_channel_, ret);
  grab_track(track.ytrack(), utime, trace_y_channel_, ret);
}

void mo01_nmx::grab_track(const flatbuffers::Vector<flatbuffers::Offset<pos> > *data,
                          uint64_t utime, int16_t chan, SpillPtr ret)
{
  for (size_t i=0; i < data->Length(); ++i)
  {
    Event e(chan, trace_model_);
    e.set_native_time(utime);
    auto element = data->Get(i);
    e.set_value(0, element->strip());
    e.set_value(1, element->time());
    e.set_value(2, element->adc());
    ret->events.push_back(e);
  }
}

std::string mo01_nmx::debug(const GEMHist& hist)
{
  std::stringstream ss;
  if (hist.xstrips()->Length())
    ss << "  strips_x: " << print_hist(hist.xstrips()) << "\n";
  if (hist.ystrips()->Length())
    ss << "  strips_y: " << print_hist(hist.ystrips()) << "\n";
  if (hist.xspectrum()->Length())
    ss << "  adc_x: " << print_hist(hist.xspectrum()) << "\n";
  if (hist.yspectrum()->Length())
    ss << "  adc_y: " << print_hist(hist.yspectrum()) << "\n";
  if (hist.cluster_spectrum()->Length())
    ss << "  adc_cluster: " << print_hist(hist.cluster_spectrum()) << "\n";
  return ss.str();
}

std::string mo01_nmx::print_hist(const flatbuffers::Vector<uint32_t>* data)
{
  std::stringstream ss;
  for (size_t i=0; i < data->Length(); ++i)
    ss << " " << data->Get(i);
  return ss.str();
}

std::string mo01_nmx::debug(const GEMTrack& track)
{
  std::stringstream ss;
  if (track.xtrack()->Length())
    ss << "  x: " << print_track(track.xtrack()) << "\n";
  if (track.ytrack()->Length())
    ss << "  y: " << print_track(track.ytrack()) << "\n";
  return ss.str();
}

std::string mo01_nmx::print_track(const flatbuffers::Vector<flatbuffers::Offset<pos>>* data)
{
  std::stringstream ss;
  for (size_t i=0; i < data->Length(); ++i)
  {
    auto element = data->Get(i);
    ss << "(" << element->strip()
       << "," << element->time()
       << ")=" << element->adc()
       << " ";
  }
  return ss.str();
}
