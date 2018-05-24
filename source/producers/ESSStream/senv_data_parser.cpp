#include "senv_data_parser.h"

#include "custom_timer.h"
#include "custom_logger.h"

SenvParser::SenvParser()
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

StreamManifest SenvParser::stream_manifest() const
{
  StreamManifest ret;
  ret[stream_id_].event_model = event_model_;
  ret[stream_id_].stats.branches.add(SettingMeta("native_time", SettingType::precise));
  ret[stream_id_].stats.branches.add(SettingMeta("dropped_buffers", SettingType::precise));
  return ret;
}

Setting SenvParser::settings() const
{
  std::string r{plugin_name()};
  auto set = get_rich_setting(r);

  set.set(Setting::text(r + "/EventsStream", stream_id_));
  
  set.branches.add_a(TimeBasePlugin(event_model_.timebase).settings());

  set.enable_if_flag(!(status_ & booted), "preset");
  return set;
}

void SenvParser::settings(const Setting& settings)
{
  std::string r{plugin_name()};
  auto set = enrich_and_toggle_presets(settings);
  stream_id_ = set.find({r + "/EventsStream"}).get_text();

  TimeBasePlugin tbs;
  tbs.settings(set.find({tbs.plugin_name()}));
  event_model_.timebase = tbs.timebase();
}

uint64_t SenvParser::stop(SpillQueue spill_queue)
{
  if (started_)
    {
    auto ret = std::make_shared<Spill>(stream_id_, StatusType::stop);
    ret->state.branches.add(Setting::precise("native_time", stats.time_end));
    ret->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));

    spill_queue->enqueue(ret);
    
    started_ = false;
    return 1;
    }
  
  return 0;
}

uint64_t SenvParser::process_payload(SpillQueue spill_queue, void* msg) {
  CustomTimer timer(true);
  uint64_t pushed_spills = 1;
  boost::posix_time::ptime start_time {boost::posix_time::microsec_clock::universal_time()};
  
  auto SenvParserTimeStamp = GetLogData(msg);
  
  stats.time_start = stats.time_end = SenvParserTimeStamp->timestamp();
  
  auto ret = std::make_shared<Spill>(stream_id_, StatusType::running);
  ret->state.branches.add(Setting::precise("native_time", SenvParserTimeStamp->timestamp()));
  ret->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
  ret->event_model = event_model_;
  ret->events.reserve(1, event_model_);
  
  auto& e = ret->events.last();
  e.set_time(SenvParserTimeStamp->timestamp());
  
  std::string TempName = SenvParserTimeStamp->source_name()->c_str();
  if (PVNameMap.find(TempName) == PVNameMap.end()) {
    PVNameMap[TempName] = PVNameMap.size();
  }
  
  e.set_value(0, PVNameMap[TempName]);
  
  ++ ret->events;
  ret->events.finalize();

  if (!started_)
  {
    auto start_spill = std::make_shared<Spill>(stream_id_, StatusType::start);
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

std::string SenvParser::debug(const LogData& TDCTimeStamp)
{
  std::stringstream ss;
  ss << "  Name      : " << TDCTimeStamp.source_name()->c_str() << "\n";
  ss << "  Timestamp : " << TDCTimeStamp.timestamp() << "\n";
  return ss.str();
}

