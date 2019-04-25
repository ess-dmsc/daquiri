#include <producers/ESSStream/f142_parser.h>

#include <core/util/timer.h>
#include <core/util/logger.h>

ChopperTDC::ChopperTDC()
    : fb_parser()
{
  std::string r{plugin_name()};

  SettingMeta fsname(r + "/FilterSourceName", SettingType::boolean, "Filter on source name");
  fsname.set_flag("preset");
  add_definition(fsname);

  SettingMeta sname(r + "/SourceName", SettingType::text, "Source name");
  sname.set_flag("preset");
  add_definition(sname);

  SettingMeta
      chopperTDCStreamid(r + "/EventsStream", SettingType::text, "DAQuiri stream ID for Chopper TDC time stamps");
  chopperTDCStreamid.set_flag("preset");
  add_definition(chopperTDCStreamid);

  int32_t i{0};
  SettingMeta root(r, SettingType::stem);
  root.set_flag("producer");
  root.set_enum(i++, r + "/EventsStream");
  root.set_enum(i++, r + "/FilterSourceName");
  root.set_enum(i++, r + "/SourceName");
  add_definition(root);

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

StreamManifest ChopperTDC::stream_manifest() const
{
  StreamManifest ret;
  ret[stream_id_].event_model = event_model_;
  ret[stream_id_].stats.branches.add(SettingMeta("native_time", SettingType::precise));
  ret[stream_id_].stats.branches.add(SettingMeta("dropped_buffers", SettingType::precise));
  return ret;
}

Setting ChopperTDC::settings() const
{
  std::string r{plugin_name()};
  auto set = get_rich_setting(r);

  set.set(Setting::text(r + "/EventsStream", stream_id_));
  set.set(Setting::boolean(r + "/FilterSourceName", filter_source_name_));
  set.set(Setting::text(r + "/SourceName", source_name_));

  set.branches.add_a(TimeBasePlugin(event_model_.timebase).settings());

  set.enable_if_flag(!(status_ & booted), "preset");
  return set;
}

void ChopperTDC::settings(const Setting& settings)
{
  std::string r{plugin_name()};
  auto set = enrich_and_toggle_presets(settings);
  stream_id_ = set.find({r + "/EventsStream"}).get_text();
  filter_source_name_ = set.find({r + "/FilterSourceName"}).triggered();
  source_name_ = set.find({r + "/SourceName"}).get_text();

  TimeBasePlugin tbs;
  tbs.settings(set.find({tbs.plugin_name()}));
  event_model_.timebase = tbs.timebase();
}

uint64_t ChopperTDC::stop(SpillQueue spill_queue)
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

std::string ChopperTDC::schema_id() const
{
  return std::string(LogDataIdentifier());
}

std::string ChopperTDC::get_source_name(void* msg) const
{
  auto em = GetLogData(msg);
  auto NamePtr = em->source_name();
  if (NamePtr == nullptr)
  {
    ERR("<ChopperTDC> message has no source_name");
    return "";
  }
  return NamePtr->str();
}

uint64_t ChopperTDC::process_payload(SpillQueue spill_queue, void* msg)
{
  Timer timer(true);
  uint64_t pushed_spills = 1;
  hr_time_t start_time{std::chrono::system_clock::now()};

  auto ChopperTDCTimeStamp = GetLogData(msg);

  std::string source_name = ChopperTDCTimeStamp->source_name()->str();
  if (filter_source_name_ && (source_name_ != source_name))
  {
    stats.time_spent += timer.s();
    return 0;
  }

  stats.time_start = stats.time_end = ChopperTDCTimeStamp->timestamp();

  auto ret = std::make_shared<Spill>(stream_id_, Spill::Type::running);
  ret->state.branches.add(Setting::precise("native_time", ChopperTDCTimeStamp->timestamp()));
  ret->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
  ret->event_model = event_model_;
  ret->events.reserve(1, event_model_);

  auto& e = ret->events.last();
  e.set_time(ChopperTDCTimeStamp->timestamp());
  ++ret->events;
  ret->events.finalize();

  if (!started_)
  {
    auto start_spill = std::make_shared<Spill>(stream_id_, Spill::Type::start);
    start_spill->time = start_time;
    start_spill->state.branches.add(Setting::precise("native_time", stats.time_start));
    start_spill->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
    spill_queue->enqueue(start_spill);
    started_ = true;
    pushed_spills++;
  }

  spill_queue->enqueue(ret);

  stats.time_spent = timer.s();
  return pushed_spills;
}

std::string ChopperTDC::debug(const LogData& TDCTimeStamp)
{
  std::stringstream ss;
  ss << "  Name      : " << TDCTimeStamp.source_name()->str() << "\n";
  ss << "  Timestamp : " << TDCTimeStamp.timestamp() << "\n";
  return ss.str();
}

