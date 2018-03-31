#include "f142_parser.h"

#include "custom_timer.h"
#include "custom_logger.h"

ChopperTDC::ChopperTDC()
: fb_parser()
{
  std::string r{plugin_name()};

  SettingMeta chopperTDCStreamid(r + "/EventsStream", SettingType::text, "DAQuiri stream ID for Chopper TDC time stamps");
  chopperTDCStreamid.set_flag("preset");
  add_definition(chopperTDCStreamid);
  
  SettingMeta tm(r + "/TimebaseMult", SettingType::integer, "Timebase multiplier");
  tm.set_val("min", 1);
  tm.set_val("units", "ns");
  add_definition(tm);
  
  SettingMeta td(r + "/TimebaseDiv", SettingType::integer, "Timebase divider");
  td.set_val("min", 1);
  td.set_val("units", "1/ns");
  add_definition(td);
  
  SettingMeta root(r, SettingType::stem);
  root.set_flag("producer");
  root.set_enum(0, r + "/EventsStream");
  root.set_enum(1, r + "/TimebaseMult");
  root.set_enum(2, r + "/TimebaseDiv");
  add_definition(root);
  
  event_model_.add_value("chopper", 0);
  
  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

StreamManifest ChopperTDC::stream_manifest() const
{
  StreamManifest ret;
  ret[stream_id_] = event_model_;
  return ret;
}

Setting ChopperTDC::settings() const
{
  std::string r{plugin_name()};
  auto set = get_rich_setting(r);

  set.set(Setting::text(r + "/EventsStream", stream_id_));
  
  set.set(Setting::integer(r + "/TimebaseMult",
                           event_model_.timebase.multiplier()));
  set.set(Setting::integer(r + "/TimebaseDiv",
                           event_model_.timebase.divider()));

  set.enable_if_flag(!(status_ & booted), "preset");
  return set;
}

void ChopperTDC::settings(const Setting& settings)
{
  std::string r{plugin_name()};
  auto set = enrich_and_toggle_presets(settings);
  stream_id_ = set.find({r + "/EventsStream"}).get_text();
  
  uint32_t mult = set.find({r + "/TimebaseMult"}).get_number();
  uint32_t div = set.find({r + "/TimebaseDiv"}).get_number();
  
  event_model_.timebase = TimeBase(mult ? mult : 1, div ? div : 1);
}

uint64_t ChopperTDC::stop(SpillQueue spill_queue)
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

uint64_t ChopperTDC::process_payload(SpillQueue spill_queue, void* msg) {
  CustomTimer timer(true);
  uint64_t pushed_spills = 1;
  boost::posix_time::ptime start_time {boost::posix_time::microsec_clock::universal_time()};
  
  auto ChopperTDCTimeStamp = GetLogData(msg);
  
  stats.time_start = stats.time_end = ChopperTDCTimeStamp->timestamp();
  
  auto ret = std::make_shared<Spill>(stream_id_, StatusType::running);
  ret->state.branches.add(Setting::precise("native_time", ChopperTDCTimeStamp->timestamp()));
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
    auto start_spill = std::make_shared<Spill>(stream_id_, StatusType::start);
    start_spill->time = start_time;
    start_spill->state.branches.add(Setting::precise("native_time", stats.time_start));
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

