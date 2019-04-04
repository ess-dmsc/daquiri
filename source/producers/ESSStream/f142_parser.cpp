#include <producers/ESSStream/f142_parser.h>

#include <core/util/timer.h>
#include <core/util/logger.h>

ChopperTDC::ChopperTDC()
: fb_parser()
{
  std::string r{plugin_name()};

  SettingMeta chopperTDCStreamid(r + "/EventsStream", SettingType::text, "DAQuiri stream ID for Chopper TDC time stamps");
  chopperTDCStreamid.set_flag("preset");
  add_definition(chopperTDCStreamid);

  int32_t i{0};
  SettingMeta root(r, SettingType::stem);
  root.set_flag("producer");
  root.set_enum(i++, r + "/EventsStream");
  add_definition(root);
  
  event_model_.add_value("chopper", 0);
  
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
  
  set.branches.add_a(TimeBasePlugin(event_model_.timebase).settings());

  set.enable_if_flag(!(status_ & booted), "preset");
  return set;
}

void ChopperTDC::settings(const Setting& settings)
{
  std::string r{plugin_name()};
  auto set = enrich_and_toggle_presets(settings);
  stream_id_ = set.find({r + "/EventsStream"}).get_text();

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

uint64_t ChopperTDC::process_payload(SpillQueue spill_queue, void* msg) {
  Timer timer(true);
  uint64_t pushed_spills = 1;
  hr_time_t start_time {std::chrono::system_clock::now()};
  
  auto ChopperTDCTimeStamp = GetLogData(msg);
  
  stats.time_start = stats.time_end = ChopperTDCTimeStamp->timestamp();
  
  auto ret = std::make_shared<Spill>(stream_id_, Spill::Type::running);
  ret->state.branches.add(Setting::precise("native_time", ChopperTDCTimeStamp->timestamp()));
  ret->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
  ret->event_model = event_model_;
  ret->events.reserve(1, event_model_);
  
  auto& e = ret->events.last();
  e.set_time(ChopperTDCTimeStamp->timestamp());
  
  std::string TempName = ChopperTDCTimeStamp->source_name()->c_str();
  if (PVNameMap.find(TempName) == PVNameMap.end()) {
    PVNameMap[TempName] = PVNameMap.size();
  }
  
  e.set_value(0, PVNameMap[TempName]);
  
  ++ ret->events;
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
  ss << "  Name      : " << TDCTimeStamp.source_name()->c_str() << "\n";
  ss << "  Timestamp : " << TDCTimeStamp.timestamp() << "\n";
  return ss.str();
}

