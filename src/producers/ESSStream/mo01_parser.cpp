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

  hists_model_.add_trace("histx", {1500});
  hists_model_.add_trace("histy", {1500});

  trace_model_.add_value("strip", 0);
  trace_model_.add_value("time", 0);
  trace_model_.add_value("adc", 16);

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
    ret = Spill::make_new(StatusType::running, {hists_channel_});
    ret->stats[hists_channel_].set_model(hists_model_);

    produce_hists(*em->data_as_GEMHist(), ret);
  }

  if (type == DataField::GEMTrack)
  {
    ret = Spill::make_new(StatusType::running,
    {trace_x_channel_, trace_y_channel_});
    ret->stats[trace_x_channel_].set_model(trace_model_);
    ret->stats[trace_y_channel_].set_model(trace_model_);

    produce_tracks(*em->data_as_GEMTrack(), ret);
  }

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
    if (hist->xhist()->Length())
      return false;
    if (hist->yhist()->Length())
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

void mo01_nmx::produce_hists(const GEMHist& hist, SpillPtr ret)
{
//  DBG << "Received GEMHist\n" << debug(hist);

  Event e(hists_channel_, hists_model_);

  auto xhist = hist.xhist();
  if (xhist->Length())
  {
    std::vector<uint16_t> vals(xhist->Length(), 0);
    for (size_t i=0; i < xhist->Length(); ++i)
      vals[i] = xhist->Get(i);
    e.set_trace(0, vals);
  }

  auto yhist = hist.yhist();
  if (yhist->Length())
  {
    std::vector<uint16_t> vals(yhist->Length(), 0);
    for (size_t i=0; i < yhist->Length(); ++i)
      vals[i] = yhist->Get(i);
    e.set_trace(1, vals);
  }

  ret->events.push_back(e);
}

std::string mo01_nmx::debug(const GEMHist& hist)
{
  std::stringstream ss;

  auto xhist = hist.xhist();
  if (xhist->Length())
  {
    ss << "  x: ";
    for (size_t i=0; i < xhist->Length(); ++i)
      ss << " " << xhist->Get(i);
    ss << "\n";
  }

  auto yhist = hist.yhist();
  if (yhist->Length())
  {
    ss << "  y: ";
    for (size_t i=0; i < yhist->Length(); ++i)
      ss << " " << yhist->Get(i);
    ss << "\n";
  }

  return ss.str();
}

void mo01_nmx::produce_tracks(const GEMTrack& track, SpillPtr ret)
{
//  DBG << "Received GEMTrack\n" << debug(track);

  auto xtrack = track.xtrack();
  for (size_t i=0; i < xtrack->Length(); ++i)
  {
    Event e(trace_x_channel_, trace_model_);
    auto element = xtrack->Get(i);
    e.set_value(0, element->strip());
    e.set_value(1, element->time());
    e.set_value(2, element->adc());
    ret->events.push_back(e);
  }

  auto ytrack = track.ytrack();
  for (size_t i=0; i < ytrack->Length(); ++i)
  {
    Event e(trace_y_channel_, trace_model_);
    auto element = ytrack->Get(i);
    e.set_value(0, element->strip());
    e.set_value(1, element->time());
    e.set_value(2, element->adc());
    ret->events.push_back(e);
  }
}


std::string mo01_nmx::debug(const GEMTrack& track)
{
  std::stringstream ss;

  auto xtrack = track.xtrack();
  if (xtrack->Length())
  {
    ss << "  x: ";
    for (size_t i=0; i < xtrack->Length(); ++i)
    {
      auto element = xtrack->Get(i);
      ss << "(" << element->strip()
         << "," << element->time()
         << ")=" << element->adc()
         << " ";
    }
    ss << "\n";
  }

  auto ytrack = track.ytrack();
  if (ytrack->Length())
  {
    ss << "  y: ";
    for (size_t i=0; i < ytrack->Length(); ++i)
    {
      auto element = ytrack->Get(i);
      ss << "(" << element->strip()
         << "," << element->time()
         << ")=" << element->adc()
         << " ";
    }
    ss << "\n";
  }

  return ss.str();
}
