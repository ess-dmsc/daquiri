#include <producers/ESSStream/mo01_parser.h>

#include <core/util/timer.h>
#include <core/util/logger.h>

mo01_nmx::mo01_nmx()
  : fb_parser()
{
  std::string r{plugin_name()};

  SettingMeta hstreamid(r + "/HistsStream", SettingType::text, "DAQuiri stream ID for prebinned histograms");
  hstreamid.set_flag("preset");
  add_definition(hstreamid);

  SettingMeta xstreamid(r + "/XStream", SettingType::text, "DAQuiri stream ID for X track");
  xstreamid.set_flag("preset");
  add_definition(xstreamid);

  SettingMeta ystreamid(r + "/YStream", SettingType::text, "DAQuiri stream ID for Y track");
  ystreamid.set_flag("preset");
  add_definition(ystreamid);

  SettingMeta hitstreamid(r + "/HitsStream", SettingType::text, "DAQuiri stream ID for hits");
  hitstreamid.set_flag("preset");
  add_definition(hitstreamid);

  int32_t i{0};
  SettingMeta root(r, SettingType::stem);
  root.set_flag("producer");
  root.set_enum(i++, r + "/HistsStream");
  root.set_enum(i++, r + "/XStream");
  root.set_enum(i++, r + "/YStream");
  root.set_enum(i++, r + "/HitsStream");
  add_definition(root);

  hists_model_.add_trace("strips_x", {UINT16_MAX+1});
  hists_model_.add_trace("strips_y", {UINT16_MAX+1});
  hists_model_.add_trace("adc_x", {UINT16_MAX+1});
  hists_model_.add_trace("adc_y", {UINT16_MAX+1});
  hists_model_.add_trace("adc_cluster", {UINT16_MAX+1});

  track_model_.add_value("strip", 0);
  track_model_.add_value("time", 0);
  track_model_.add_value("adc", 0);

  hits_model_.add_value("plane", 0);
  hits_model_.add_value("channel", 0);
  hits_model_.add_value("adc", 0);

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

Setting mo01_nmx::settings() const
{
  std::string r{plugin_name()};
  auto set = get_rich_setting(r);

  set.set(Setting::text(r + "/HistsStream", hists_stream_id_));
  set.set(Setting::text(r + "/XStream", x_stream_id_));
  set.set(Setting::text(r + "/YStream", y_stream_id_));
  set.set(Setting::text(r + "/HitsStream", hit_stream_id_));

  set.branches.add_a(TimeBasePlugin(hists_model_.timebase).settings());

  set.enable_if_flag(!(status_ & booted), "preset");
  return set;
}

void mo01_nmx::settings(const Setting& settings)
{
  std::string r{plugin_name()};

  auto set = enrich_and_toggle_presets(settings);
  hists_stream_id_ = set.find({r + "/HistsStream"}).get_text();
  x_stream_id_ = set.find({r + "/XStream"}).get_text();
  y_stream_id_ = set.find({r + "/YStream"}).get_text();
  hit_stream_id_ = set.find({r + "/HitsStream"}).get_text();

  TimeBasePlugin tbs;
  tbs.settings(set.find({tbs.plugin_name()}));
  hists_model_.timebase = tbs.timebase();
}

StreamManifest mo01_nmx::stream_manifest() const
{
  StreamManifest ret;
  ret[hists_stream_id_].event_model = hists_model_;
  ret[hists_stream_id_].stats.branches.add(SettingMeta("native_time", SettingType::precise));
  ret[hists_stream_id_].stats.branches.add(SettingMeta("dropped_buffers", SettingType::precise));

  ret[x_stream_id_].event_model = track_model_;
  ret[x_stream_id_].stats.branches.add(SettingMeta("native_time", SettingType::precise));
  ret[x_stream_id_].stats.branches.add(SettingMeta("dropped_buffers", SettingType::precise));

  ret[y_stream_id_].event_model = track_model_;
  ret[y_stream_id_].stats.branches.add(SettingMeta("native_time", SettingType::precise));
  ret[y_stream_id_].stats.branches.add(SettingMeta("dropped_buffers", SettingType::precise));

  ret[hit_stream_id_].event_model = hits_model_;
  ret[hit_stream_id_].stats.branches.add(SettingMeta("native_time", SettingType::precise));
  ret[hit_stream_id_].stats.branches.add(SettingMeta("dropped_buffers", SettingType::precise));
  return ret;
}

uint64_t mo01_nmx::stop(SpillQueue spill_queue)
{
  if (started_)
  {
    auto ret = std::make_shared<Spill>(hists_stream_id_, Spill::Type::stop);
    ret->state.branches.add(Setting::precise("native_time", spoofed_time_));
    ret->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
    spill_queue->enqueue(ret);

    auto ret2 = std::make_shared<Spill>(x_stream_id_, Spill::Type::stop);
    ret2->state.branches.add(Setting::precise("native_time", spoofed_time_));
    ret2->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
    spill_queue->enqueue(ret2);

    auto ret3 = std::make_shared<Spill>(y_stream_id_, Spill::Type::stop);
    ret3->state.branches.add(Setting::precise("native_time", spoofed_time_));
    ret3->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
    spill_queue->enqueue(ret3);

    auto ret4 = std::make_shared<Spill>(hit_stream_id_, Spill::Type::stop);
    ret4->state.branches.add(Setting::precise("native_time", spoofed_time_));
    ret4->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
    spill_queue->enqueue(ret4);

    started_ = false;
    return 3;
  }

  return 0;
}

uint64_t mo01_nmx::process_payload(SpillQueue spill_queue, void* msg)
{
  Timer timer(true);
  uint64_t pushed_spills = 0;

  auto em = GetMonitorMessage(msg);

  spoofed_time_++;

  if (!started_)
  {
    auto ret = std::make_shared<Spill>(hists_stream_id_, Spill::Type::start);
    ret->state.branches.add(Setting::precise("native_time", spoofed_time_));
    ret->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
    spill_queue->enqueue(ret);

    auto ret2 = std::make_shared<Spill>(x_stream_id_, Spill::Type::start);
    ret2->state.branches.add(Setting::precise("native_time", spoofed_time_));
    ret2->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
    spill_queue->enqueue(ret2);

    auto ret3 = std::make_shared<Spill>(y_stream_id_, Spill::Type::start);
    ret3->state.branches.add(Setting::precise("native_time", spoofed_time_));
    ret3->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
    spill_queue->enqueue(ret3);

    auto ret4 = std::make_shared<Spill>(hit_stream_id_, Spill::Type::start);
    ret4->state.branches.add(Setting::precise("native_time", spoofed_time_));
    ret4->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
    spill_queue->enqueue(ret4);

    started_ = true;
    pushed_spills += 3;
  }

  auto type = em->data_type();
  if (type == DataField::GEMHist)
    pushed_spills += produce_hists(*reinterpret_cast<const GEMHist*>(em->data()), spill_queue);
  else if (type == DataField::GEMTrack)
    pushed_spills += produce_tracks(*reinterpret_cast<const GEMTrack*>(em->data()), spill_queue);
  else if (type == DataField::MONHit)
    pushed_spills += produce_hits(*reinterpret_cast<const MONHit*>(em->data()), spill_queue);

  stats.time_spent = timer.s();
  return pushed_spills;
}

uint64_t mo01_nmx::produce_hists(const GEMHist& hist, SpillQueue queue)
{
  if (!hist.xstrips()->Length() &&
      !hist.xstrips()->Length() &&
      !hist.xspectrum()->Length() &&
      !hist.yspectrum()->Length() &&
      !hist.cluster_spectrum()->Length())
    return 0;

  auto ret = std::make_shared<Spill>(hists_stream_id_, Spill::Type::running);
  ret->state.branches.add(Setting::precise("native_time", spoofed_time_));
  ret->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
  ret->event_model = hists_model_;
  ret->events.reserve(1, hists_model_);

//  DBG( "Received GEMHist\n" << debug(hist);

  auto& e = ret->events.last();
  e.set_time(spoofed_time_);

  grab_hist(e, 0, hist.xstrips());
  grab_hist(e, 1, hist.ystrips());
  grab_hist(e, 2, hist.xspectrum());
  grab_hist(e, 3, hist.yspectrum());
  grab_hist(e, 4, hist.cluster_spectrum());

  ++ ret->events;
  ret->events.finalize();

//  DBG( "Will enqueue with model " << ret->event_model.debug();

  queue->enqueue(ret);
  return 1;
}

uint64_t mo01_nmx::produce_tracks(const GEMTrack& track, SpillQueue queue)
{
  uint64_t pushed_spills {0};

  if (track.xtrack()->Length())
  {
    queue->enqueue(grab_track(track.xtrack(), x_stream_id_));
    pushed_spills ++;
  }

  if (track.ytrack()->Length())
  {
    queue->enqueue(grab_track(track.ytrack(), y_stream_id_));
    pushed_spills ++;
  }

  return pushed_spills;
}

uint64_t mo01_nmx::produce_hits(const MONHit& hits, SpillQueue queue)
{
  auto spill = std::make_shared<Spill>(hit_stream_id_, Spill::Type::running);
  spill->state.branches.add(Setting::precise("native_time", spoofed_time_));
  spill->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
  spill->event_model = hits_model_;
  spill->events.reserve(hits.plane()->Length(), hits_model_);

  for (size_t i=0; i < hits.plane()->Length(); ++i)
  {
    auto& e = spill->events.last();
    e.set_time(spoofed_time_ + hits.time()->Get(i));
    e.set_value(0, hits.plane()->Get(i));
    e.set_value(1, hits.channel()->Get(i));
    e.set_value(2, hits.adc()->Get(i));
    ++ spill->events;
  }

  spill->events.finalize();

  if (hits.plane()->Length())
  {
    queue->enqueue(spill);
    return 1;
  }

  return 0;
}

void mo01_nmx::grab_hist(Event& e, size_t idx, const flatbuffers::Vector<uint32_t>* data)
{
  if (!data->Length())
    return;
//  std::vector<uint32_t> vals(data->Length(), 0);
  auto& trace = e.trace(idx);
  for (size_t i=0; i < data->Length(); ++i)
    trace[i] = data->Get(i);
//  DBG( "Added hist " << idx << " length " << data->Length();
}

SpillPtr mo01_nmx::grab_track(const flatbuffers::Vector<flatbuffers::Offset<pos>>* data,
                              std::string stream)
{
  auto ret = std::make_shared<Spill>(stream, Spill::Type::running);

  ret->state.branches.add(Setting::precise("native_time", spoofed_time_));
  ret->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
  ret->event_model = track_model_;
  ret->events.reserve(data->Length(), track_model_);

  for (size_t i=0; i < data->Length(); ++i)
  {
    auto& e = ret->events.last();
    e.set_time(spoofed_time_);
    const auto& element = data->Get(i);
    e.set_value(0, element->strip());
    e.set_value(1, element->time());
    e.set_value(2, element->adc());
    ++ ret->events;
  }
  ret->events.finalize();

  return ret;
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
