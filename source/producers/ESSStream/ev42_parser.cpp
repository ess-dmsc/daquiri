#include <producers/ESSStream/ev42_parser.h>
#include "ev42_events_generated.h"

#include <core/util/timer.h>
#include <core/util/logger.h>

ev42_events::ev42_events()
{
  std::string r{plugin_name()};

  SettingMeta streamid(r + "/StreamID", SettingType::text, "DAQuiri stream ID");
  streamid.set_flag("preset");
  add_definition(streamid);

  SettingMeta spoof(r + "/SpoofClock", SettingType::menu, "Spoof pulse time");
  spoof.set_enum(Spoof::None, "no");
  spoof.set_enum(Spoof::Monotonous, "monotonous high-time");
  spoof.set_enum(Spoof::Earliest, "use earliest event in buffer");
  add_definition(spoof);

  SettingMeta hb(r + "/Heartbeat", SettingType::boolean, "Send empty heartbeat buffers");
  add_definition(hb);

  SettingMeta fsname(r + "/FilterSourceName", SettingType::boolean, "Filter on source name");
  fsname.set_flag("preset");
  add_definition(fsname);

  SettingMeta sname(r + "/SourceName", SettingType::text, "Source name");
  sname.set_flag("preset");
  add_definition(sname);

  SettingMeta oor(r + "/MessageOrdering", SettingType::menu, "If message_id out of order");
  oor.set_enum(CheckOrdering::Ignore, "ignore");
  oor.set_enum(CheckOrdering::Warn, "warn");
  oor.set_enum(CheckOrdering::Reject, "reject");
  add_definition(oor);

  int32_t i{0};
  SettingMeta root(r, SettingType::stem);
  root.set_flag("producer");
  root.set_enum(i++, r + "/StreamID");
  root.set_enum(i++, r + "/SpoofClock");
  root.set_enum(i++, r + "/Heartbeat");
  root.set_enum(i++, r + "/FilterSourceName");
  root.set_enum(i++, r + "/SourceName");
  root.set_enum(i++, r + "/MessageOrdering");

  add_definition(root);
}

Setting ev42_events::settings() const
{
  std::string r{plugin_name()};
  auto set = get_rich_setting(r);

  set.set(Setting::boolean(r + "/FilterSourceName", filter_source_name_));
  set.set(Setting::text(r + "/SourceName", source_name_));
  set.set(Setting::text(r + "/StreamID", stream_id_));
  set.set(Setting::integer(r + "/SpoofClock", spoof_clock_));
  set.set(Setting::boolean(r + "/Heartbeat", heartbeat_));
  set.set(Setting::integer(r + "/MessageOrdering", ordering_));

  set.branches.add_a(geometry_.settings());
  set.branches.add_a(TimeBasePlugin(event_definition_.timebase).settings());

  set.enable_if_flag(!(status_ & booted), "preset");
  return set;
}

void ev42_events::settings(const Setting& settings)
{
  std::string r{plugin_name()};
  auto set = enrich_and_toggle_presets(settings);

  filter_source_name_ = set.find({r + "/FilterSourceName"}).triggered();
  source_name_ = set.find({r + "/SourceName"}).get_text();
  stream_id_ = set.find({r + "/StreamID"}).get_text();
  spoof_clock_ = static_cast<Spoof>(set.find({r + "/SpoofClock"}).get_int());
  heartbeat_ = set.find({r + "/Heartbeat"}).triggered();
  ordering_ = static_cast<CheckOrdering>(set.find({r + "/MessageOrdering"}).get_int());

  event_definition_ = EventModel();

  TimeBasePlugin tbs;
  tbs.settings(set.find({tbs.plugin_name()}));
  event_definition_ = EventModel();
  event_definition_.timebase = tbs.timebase();
  geometry_.settings(set.find({geometry_.plugin_name()}));
  geometry_.define(event_definition_);
}

StreamManifest ev42_events::stream_manifest() const
{
  StreamManifest ret;
  ret[stream_id_].event_model = event_definition_;
  ret[stream_id_].stats.branches.add(SettingMeta("native_time", SettingType::precise));
  ret[stream_id_].stats.branches.add(SettingMeta("dropped_buffers", SettingType::precise));
  ret[stream_id_].stats.branches.add(SettingMeta("pulse_time", SettingType::precise));
  return ret;
}

uint64_t ev42_events::stop(SpillQueue spill_queue)
{
  if (started_)
  {
    auto ret = std::make_shared<Spill>(stream_id_, Spill::Type::stop);
    ret->state.branches.add(Setting::precise("native_time", stats.time_end));
    ret->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
    spill_queue->enqueue(ret);
    started_ = false;
    return 1;
  }
  return 0;
}

std::string ev42_events::schema_id() const
{
  return std::string(EventMessageIdentifier());
}

std::string ev42_events::get_source_name(void* msg) const
{
  auto em = GetEventMessage(msg);
  auto NamePtr = em->source_name();
  if (NamePtr == nullptr)
  {
    ERR("<ev42_events> message has no source_name");
    return "";
  }
  return NamePtr->str();
}

uint64_t ev42_events::process_payload(SpillQueue spill_queue, void* msg)
{
  Timer timer(true);
  uint64_t pushed_spills = 0;
  hr_time_t start_time {std::chrono::system_clock::now()};

  auto em = GetEventMessage(msg);

  std::string source_name = em->source_name()->str();
  if (filter_source_name_ && (source_name_ != source_name))
  {
    stats.time_spent += timer.s();
    return 0;
  }

  if ((ordering_ != Ignore) && !in_order(em))
  {
    WARN("Buffer out of order ({}<={}) {}", em->message_id(), latest_buf_id_, debug(em));
    if (ordering_ == Reject)
    {
      stats.time_spent += timer.s();
      return 0;
    }
  }

  size_t event_count = events_in_buffer(em);

  uint64_t time_high = em->pulse_time();

  if (spoof_clock_ == Monotonous)
    time_high = spoofed_time_++ << 32;

  stats.time_start = stats.time_end = time_high;

  SpillPtr run_spill = std::make_shared<Spill>(stream_id_, Spill::Type::running);
  run_spill->event_model = event_definition_;
  run_spill->events.reserve(event_count, event_definition_);

  for (size_t i=0; i < event_count; ++i)
  {
    uint64_t time = em->time_of_flight()->Get(i);
    time += time_high;
    if (i==0)
      stats.time_start = time;
    stats.time_start = std::min(stats.time_start, time);
    stats.time_end = std::max(stats.time_end, time);

    auto& evt = run_spill->events.last();
    if (geometry_.fill(evt, em->detector_id()->Get(i)))
    {
      evt.set_time(time);
      ++ run_spill->events;
    }
    else
    {
      WARN("Out of range Pixid={}", em->detector_id()->Get(i));
    }
//    DBG( "Time " << stats.time_start << " - " << stats.time_end;
  }
  run_spill->events.finalize();

  run_spill->state.branches.add(Setting::precise("native_time", stats.time_end));
  run_spill->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));

  if (spoof_clock_ == Monotonous)
    run_spill->state.branches.add(Setting::precise("pulse_time", time_high));
  else if (spoof_clock_ == Earliest)
    run_spill->state.branches.add(Setting::precise("pulse_time", stats.time_start));
  else
    run_spill->state.branches.add(Setting::precise("pulse_time", em->pulse_time()));

  run_spill->state.branches.add(Setting::text("source_name", source_name));

  if (!started_)
  {
    auto start_spill = std::make_shared<Spill>(stream_id_, Spill::Type::start);
    start_spill->time = start_time;
    start_spill->state.branches.add(Setting::precise("native_time", time_high));
//    start_spill->state.branches.add(Setting::text("source_name", source_name));
    spill_queue->enqueue(start_spill);
    started_ = true;
    pushed_spills++;
  }

  if (event_count || heartbeat_)
  {
    spill_queue->enqueue(run_spill);
    pushed_spills++;
  }

  stats.time_spent += timer.s();
  return pushed_spills;
}

size_t ev42_events::events_in_buffer(const EventMessage* em)
{
  auto t_len = em->time_of_flight()->Length();
  auto p_len = em->detector_id()->Length();
  if ((t_len != p_len) || !t_len)
    return 0;

  return t_len;
}

bool ev42_events::in_order(const EventMessage* em)
{
  const auto& buf_id = em->message_id();
  if (buf_id <= latest_buf_id_)
    return false;
  latest_buf_id_ = std::max(latest_buf_id_, buf_id);
  return true;
}

std::string ev42_events::debug(const EventMessage* em)
{
  std::stringstream ss;

  ss << em->source_name()->str()
     << " #" << em->message_id()
     << " time=" << em->pulse_time()
     << " tof_size=" << em->time_of_flight()->Length()
     << " det_size=" << em->detector_id()->Length();

  return ss.str();
}
