#include <producers/ESSStream/senv_data_parser.h>
#include "senv_data_generated.h"

#include <core/util/timer.h>
#include <core/util/custom_logger.h>

SenvParser::SenvParser()
: fb_parser()
{
  std::string r{plugin_name()};

  SettingMeta chopperTDCStreamid(r + "/EventsStream", SettingType::text, "DAQuiri stream ID");
  chopperTDCStreamid.set_flag("preset");
  add_definition(chopperTDCStreamid);

  int32_t i{0};
  SettingMeta root(r, SettingType::stem);
  root.set_flag("producer");
  root.set_enum(i++, r + "/EventsStream");
  add_definition(root);
  
  event_model_.add_value("channel", 3);
  event_model_.add_value("value", 65535);
  event_model_.add_value("time", 0);

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

StreamManifest SenvParser::stream_manifest() const
{
  StreamManifest ret;
  ret[stream_id_].event_model = event_model_;
  ret[stream_id_].stats.branches.add(SettingMeta("native_time", SettingType::precise));
  ret[stream_id_].stats.branches.add(SettingMeta("dropped_buffers", SettingType::precise));
  ret[stream_id_].stats.branches.add(SettingMeta("senv_name", SettingType::text));
  ret[stream_id_].stats.branches.add(SettingMeta("senv_chan", SettingType::integer));
  ret[stream_id_].stats.branches.add(SettingMeta("senv_delta", SettingType::floating));
  ret[stream_id_].stats.branches.add(SettingMeta("senv_counter", SettingType::integer));
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
    auto ret = std::make_shared<Spill>(stream_id_, Spill::Type::stop);
    ret->state.branches.add(Setting::precise("native_time", stats.time_end));
    ret->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));

    spill_queue->enqueue(ret);
    
    started_ = false;
    return 1;
    }
  
  return 0;
}

uint64_t SenvParser::process_payload(SpillQueue spill_queue, void* msg) {
  Timer timer(true);
  uint64_t pushed_spills = 1;
  hr_time_t start_time {std::chrono::system_clock::now()};

  auto Data = GetSampleEnvironmentData(msg);
//  INFO("\n{}", debug(Data));
  
  stats.time_start = stats.time_end = Data->PacketTimestamp();
  auto name = Data->Name()->str();
  auto channel = Data->Channel();
  auto delta = Data->TimeDelta();
  auto messagectr = Data->MessageCounter();
  // \todo delta

  auto run_spill = std::make_shared<Spill>(stream_id_, Spill::Type::running);
  run_spill->state.branches.add(Setting::precise("native_time", Data->PacketTimestamp()));
  run_spill->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
  run_spill->state.branches.add(Setting::text("senv_name", name));
  run_spill->state.branches.add(Setting::integer("senv_chan", channel));
  run_spill->state.branches.add(Setting::floating("senv_delta", delta));
  run_spill->state.branches.add(Setting::integer("senv_counter", messagectr));

  size_t event_count = Data->Values()->size();

  if (event_count)
  {
    run_spill->event_model = event_model_;
    run_spill->events.reserve(event_count, event_model_);

    bool timestamps_included = (Data->Values()->size() == Data->Timestamps()->size());

    for (size_t i = 0; i < event_count; ++i)
    {
      auto& evt = run_spill->events.last();

      evt.set_time(Data->PacketTimestamp());
      evt.set_value(0, channel);
      evt.set_value(1, Data->Values()->Get(i));
      if (timestamps_included)
        evt.set_value(2, Data->Timestamps()->Get(i));

      ++run_spill->events;
    }
    run_spill->events.finalize();
  }

  if (!started_)
  {
    auto start_spill = std::make_shared<Spill>(stream_id_, Spill::Type::start);
    start_spill->time = start_time;
    start_spill->state.branches.add(Setting::precise("native_time", stats.time_start));
    start_spill->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
    start_spill->state.branches.add(Setting::text("senv_name", name));
    start_spill->state.branches.add(Setting::integer("senv_chan", channel));
    start_spill->state.branches.add(Setting::floating("senv_delta", delta));
    start_spill->state.branches.add(Setting::integer("senv_counter", messagectr));
    spill_queue->enqueue(start_spill);
    started_ = true;
    pushed_spills++;
  }
  
  spill_queue->enqueue(run_spill);
  
  stats.time_spent = timer.s();
  return pushed_spills;
}

std::string SenvParser::debug(const SampleEnvironmentData* Data)
{
  std::stringstream ss;

  ss << "  Name      : " << Data->Name()->str() << "\n";
  ss << "  Channel   : " << Data->Channel() << "\n";
  ss << "  PacketTs  : " << Data->PacketTimestamp() << "\n";
  ss << "  TimeDelta : " << Data->TimeDelta() << "\n";
  // \todo loctation
  ss << "  Message#  : " << Data->MessageCounter() << "\n";
  ss << "  NumValues  : " << Data->Values()->size() << "\n";
  ss << "  NumTSs     : " << Data->Timestamps()->size() << "\n";

  return ss.str();
}

