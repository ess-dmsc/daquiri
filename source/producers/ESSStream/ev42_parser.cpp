#include "ev42_parser.h"
#include "ev42_events_generated.h"

#include "custom_timer.h"
#include "custom_logger.h"

ev42_events::ev42_events()
{
  std::string mp {"ev42_events/"};

  SettingMeta streamid(mp + "StreamID", SettingType::text, "DAQuiri stream ID");
  streamid.set_flag("preset");
  add_definition(streamid);

  SettingMeta ex(mp + "extent_x", SettingType::integer, "Extent X");
  ex.set_val("min", 1);
  add_definition(ex);

  SettingMeta ey(mp + "extent_y", SettingType::integer, "Extent Y");
  ey.set_val("min", 1);
  add_definition(ey);

  SettingMeta ez(mp + "extent_z", SettingType::integer, "Extent Z");
  ez.set_val("min", 1);
  add_definition(ez);

  SettingMeta ep(mp + "panels", SettingType::integer, "Panel count");
  ep.set_val("min", 1);
  add_definition(ep);


  SettingMeta tm(mp + "TimebaseMult", SettingType::integer, "Timebase multiplier");
  tm.set_val("min", 1);
  tm.set_val("units", "ns");
  add_definition(tm);

  SettingMeta td(mp + "TimebaseDiv", SettingType::integer, "Timebase divider");
  td.set_val("min", 1);
  td.set_val("units", "1/ns");
  add_definition(td);

  SettingMeta spoof(mp + "SpoofClock", SettingType::menu, "Spoof pulse time");
  spoof.set_enum(0, "no");
  spoof.set_enum(1, "monotonous high-time");
  spoof.set_enum(2, "use earliest event in buffer");
  add_definition(spoof);

  SettingMeta hb(mp + "Heartbeat", SettingType::boolean, "Send empty heartbeat buffers");
  add_definition(hb);


  SettingMeta fsname(mp + "FilterSourceName", SettingType::boolean, "Filter on source name");
  fsname.set_flag("preset");
  add_definition(fsname);

  SettingMeta sname(mp + "SourceName", SettingType::text, "Source name");
  sname.set_flag("preset");
  add_definition(sname);

  SettingMeta oor(mp + "MessageOrdering", SettingType::menu, "If message_id out of order");
  oor.set_enum(0, "ignore");
  oor.set_enum(1, "warn");
  oor.set_enum(2, "reject");
  add_definition(oor);

  SettingMeta root("ev42_events", SettingType::stem);
  root.set_flag("producer");
  root.set_enum(2, mp + "StreamID");
  root.set_enum(3, mp + "extent_x");
  root.set_enum(4, mp + "extent_y");
  root.set_enum(5, mp + "extent_z");
  root.set_enum(6, mp + "panels");
  root.set_enum(7, mp + "TimebaseMult");
  root.set_enum(8, mp + "TimebaseDiv");
  root.set_enum(9, mp + "SpoofClock");
  root.set_enum(10, mp + "Heartbeat");

  root.set_enum(20, mp + "FilterSourceName");
  root.set_enum(21, mp + "SourceName");
  root.set_enum(22, mp + "MessageOrdering");

  add_definition(root);
}

void ev42_events::read_settings_bulk(Setting &set) const
{
  set = enrich_and_toggle_presets(set);

  std::string mp {"ev42_events/"};

  set.set(Setting::boolean(mp + "FilterSourceName", filter_source_name_));
  set.set(Setting::text(mp + "SourceName", source_name_));

  set.set(Setting::text(mp + "StreamID", stream_id_));

  set.set(Setting::integer(mp + "extent_x", integer_t(geometry_.nx())));
  set.set(Setting::integer(mp + "extent_y", integer_t(geometry_.ny())));
  set.set(Setting::integer(mp + "extent_z", integer_t(geometry_.nz())));
  set.set(Setting::integer(mp + "panels", integer_t(geometry_.np())));

  set.set(Setting::integer(mp + "TimebaseMult",
                           event_definition_.timebase.multiplier()));
  set.set(Setting::integer(mp + "TimebaseDiv",
                           event_definition_.timebase.divider()));
  set.set(Setting::integer(mp + "SpoofClock", spoof_clock_));
  set.set(Setting::boolean(mp + "Heartbeat", heartbeat_));

  set.set(Setting::integer(mp + "MessageOrdering", ordering_));
}


void ev42_events::write_settings_bulk(const Setting& settings)
{
  auto set = enrich_and_toggle_presets(settings);

  std::string mp {"ev42_events/"};

  filter_source_name_ = set.find({mp + "FilterSourceName"}).triggered();
  source_name_ = set.find({mp + "SourceName"}).get_text();

  stream_id_ = set.find({mp + "StreamID"}).get_text();
  geometry_.nx(set.find({mp + "extent_x"}).get_number());
  geometry_.ny(set.find({mp + "extent_y"}).get_number());
  geometry_.nz(set.find({mp + "extent_z"}).get_number());
  geometry_.np(set.find({mp + "panels"}).get_number());

  spoof_clock_ = set.find({mp + "SpoofClock"}).get_number();
  heartbeat_ = set.find({mp + "Heartbeat"}).triggered();

  ordering_ = set.find({mp + "MessageOrdering"}).get_number();

  event_definition_ = EventModel();

  uint32_t mult = set.find({mp + "TimebaseMult"}).get_number();
  uint32_t div = set.find({mp + "TimebaseDiv"}).get_number();
  event_definition_.timebase = TimeBase(mult ? mult : 1, div ? div : 1);

  event_definition_.add_value("x", geometry_.nx());
  event_definition_.add_value("y", geometry_.ny());
  event_definition_.add_value("z", geometry_.nz());
  event_definition_.add_value("panel", geometry_.np());
}

StreamManifest ev42_events::stream_manifest() const
{
  StreamManifest ret;
  ret[stream_id_] = event_definition_;
  return ret;
}

uint64_t ev42_events::stop(SpillQueue spill_queue)
{
  if (started_)
  {
    auto ret = std::make_shared<Spill>(stream_id_, StatusType::stop);
    ret->state.branches.add(Setting::precise("native_time", stats.time_end));
    spill_queue->enqueue(ret);
    started_ = false;
    return 1;
  }
  return 0;
}

uint64_t ev42_events::process_payload(SpillQueue spill_queue, void* msg)
{
  CustomTimer timer(true);
  uint64_t pushed_spills = 0;
  boost::posix_time::ptime start_time {boost::posix_time::microsec_clock::universal_time()};

  auto em = GetEventMessage(msg);

  std::string source_name = em->source_name()->str();

  if (filter_source_name_ && (source_name_ != source_name))
  {
    stats.time_spent = timer.s();
    return 0;
  }

  if (ordering_ && !in_order(em))
  {
    WARN << "Buffer out of order (" << em->message_id()
         << "<=" << latest_buf_id_ << ") "
         << debug(em);
    if (ordering_ == 2)
    {
      stats.time_spent = timer.s();
      return 0;
    }
  }

  size_t event_count = events_in_buffer(em);

  uint64_t time_high = em->pulse_time();

  if (spoof_clock_ == 1)
    time_high = spoofed_time_++ << 32;

  stats.time_start = stats.time_end = time_high;

  SpillPtr run_spill = std::make_shared<Spill>(stream_id_, StatusType::running);
  run_spill->event_model = event_definition_;
  run_spill->events.reserve(event_count, event_definition_);

  for (size_t i=0; i < event_count; ++i)
  {
    uint64_t time = em->time_of_flight()->Get(i);
    time |= time_high;
    if (i==0)
      stats.time_start = time;
    stats.time_start = std::min(stats.time_start, time);
    stats.time_end = std::max(stats.time_end, time);

    const auto& id = em->detector_id()->Get(i);
    if (geometry_.valid_id(id)) //must be non0?
    {
      auto& evt = run_spill->events.last();
      evt.set_value(0, geometry_.x(id));
      evt.set_value(1, geometry_.y(id));
      evt.set_value(2, geometry_.z(id));
      evt.set_value(3, geometry_.p(id));
      evt.set_time(time);
      ++ run_spill->events;
    }
//    DBG << "Time " << stats.time_start << " - " << stats.time_end;
  }
  run_spill->events.finalize();

  run_spill->state.branches.add(Setting::precise("native_time", stats.time_end));

  if (spoof_clock_ == 1)
    run_spill->state.branches.add(Setting::precise("pulse_time", time_high));
  else if (spoof_clock_ == 2)
    run_spill->state.branches.add(Setting::precise("pulse_time", stats.time_start));
  else
    run_spill->state.branches.add(Setting::precise("pulse_time", em->pulse_time()));

  run_spill->state.branches.add(Setting::text("source_name", source_name));

  if (!started_)
  {
    auto start_spill = std::make_shared<Spill>(stream_id_, StatusType::start);
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

  stats.time_spent = timer.s();
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
