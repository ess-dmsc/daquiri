#include <producers/ESSStream/senv_data_parser.h>
#include "senv_data_generated.h"

#include <core/util/timer.h>
#include <core/util/logger.h>

SenvParser::SenvParser()
    : fb_parser()
{
  std::string r{plugin_name()};

  SettingMeta fsname(r + "/FilterSourceName", SettingType::boolean, "Filter on source name");
  fsname.set_flag("preset");
  add_definition(fsname);

  SettingMeta sname(r + "/SourceName", SettingType::text, "Source name");
  sname.set_flag("preset");
  add_definition(sname);

  SettingMeta sid0(r + "/StreamBase", SettingType::text,
                   "DAQuiri stream ID base (chan num appended)");
  sid0.set_flag("preset");
  add_definition(sid0);

  int32_t i{0};
  SettingMeta root(r, SettingType::stem);
  root.set_flag("producer");
  root.set_enum(i++, r + "/StreamBase");
  root.set_enum(i++, r + "/FilterSourceName");
  root.set_enum(i++, r + "/SourceName");
  add_definition(root);

  event_model_.add_value("channel", 3);
  event_model_.add_trace("wave_form", {5000});
  event_model_.add_trace("times", {5000});

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

StreamManifest SenvParser::stream_manifest() const
{
  StreamManifest ret;
  for (size_t i=0; i < 4; ++i) {
    auto sid = stream_id_base_ + std::to_string(i);
    ret[sid].event_model = event_model_;
    ret[sid].stats.branches.add(SettingMeta("native_time", SettingType::precise));
    ret[sid].stats.branches.add(SettingMeta("dropped_buffers", SettingType::precise));
    ret[sid].stats.branches.add(SettingMeta("senv_name", SettingType::text));
    ret[sid].stats.branches.add(SettingMeta("senv_chan", SettingType::integer));
    ret[sid].stats.branches.add(SettingMeta("senv_delta", SettingType::floating));
    ret[sid].stats.branches.add(SettingMeta("senv_counter", SettingType::integer));
  }
  return ret;
}

Setting SenvParser::settings() const
{
  std::string r{plugin_name()};
  auto set = get_rich_setting(r);

  set.set(Setting::text(r + "/StreamBase", stream_id_base_));
  set.set(Setting::boolean(r + "/FilterSourceName", filter_source_name_));
  set.set(Setting::text(r + "/SourceName", source_name_));

  set.branches.add_a(TimeBasePlugin(event_model_.timebase).settings());

  set.enable_if_flag(!(status_ & booted), "preset");
  return set;
}

void SenvParser::settings(const Setting& settings)
{
  std::string r{plugin_name()};
  auto set = enrich_and_toggle_presets(settings);
  stream_id_base_ = set.find({r + "/StreamBase"}).get_text();
  filter_source_name_ = set.find({r + "/FilterSourceName"}).triggered();
  source_name_ = set.find({r + "/SourceName"}).get_text();

  TimeBasePlugin tbs;
  tbs.settings(set.find({tbs.plugin_name()}));
  event_model_.timebase = tbs.timebase();
}

uint64_t SenvParser::stop(SpillQueue spill_queue)
{
  if (started_)
  {
    for (size_t i=0; i <4; ++i) {
      auto sid = stream_id_base_ + std::to_string(i);
      auto ret = std::make_shared<Spill>(sid, Spill::Type::stop);
      ret->state.branches.add(Setting::precise("native_time", stats.time_end));
      ret->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
      spill_queue->enqueue(ret);
    }
    started_ = false;
    return 4;
  }
  return 0;
}

uint64_t SenvParser::start(SpillQueue spill_queue)
{
  for (size_t i=0; i <4; ++i)
  {
    auto sid = stream_id_base_ + std::to_string(i);
    auto run_spill = std::make_shared<Spill>(sid, Spill::Type::start);
    run_spill->state.branches.add(Setting::precise("native_time", stats.time_start));
    run_spill->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
    spill_queue->enqueue(run_spill);
  }
  return 4;
}

std::string SenvParser::schema_id() const
{
  return std::string(SampleEnvironmentDataIdentifier());
}

std::string SenvParser::get_source_name(void* msg) const
{
  auto em = GetSampleEnvironmentData(msg);
  auto NamePtr = em->Name();
  if (NamePtr == nullptr)
  {
    ERR("<mo01_nmx> message has no source_name");
    return "";
  }
  return NamePtr->str();
}

uint64_t SenvParser::process_payload(SpillQueue spill_queue, void* msg)
{
  Timer timer(true);
  uint64_t pushed_spills = 0;
  hr_time_t start_time{std::chrono::system_clock::now()};

  auto Data = GetSampleEnvironmentData(msg);
//  INFO("\n{}", debug(Data));

  std::string source_name = Data->Name()->str();
  if (filter_source_name_ && (source_name_ != source_name))
  {
    stats.time_spent += timer.s();
    return 0;
  }


  stats.time_start = stats.time_end = Data->PacketTimestamp();
  auto channel = Data->Channel();
  auto delta = Data->TimeDelta();
  auto messagectr = Data->MessageCounter();
  // \todo delta

  if (!started_)
  {
    pushed_spills += start(spill_queue);
    started_ = true;
  }

  auto sid = stream_id_base_ + std::to_string(channel);
  auto run_spill = std::make_shared<Spill>(sid, Spill::Type::running);
  run_spill->state.branches.add(Setting::precise("native_time", Data->PacketTimestamp()));
  run_spill->state.branches.add(Setting::precise("dropped_buffers", stats.dropped_buffers));
  run_spill->state.branches.add(Setting::text("senv_name", source_name));
  run_spill->state.branches.add(Setting::integer("senv_chan", channel));
  run_spill->state.branches.add(Setting::floating("senv_delta", delta));
  run_spill->state.branches.add(Setting::integer("senv_counter", messagectr));

  size_t event_count = Data->Values()->size();

  if (event_count)
  {
    run_spill->event_model = event_model_;
    run_spill->events.reserve(1, event_model_);
    auto& evt = run_spill->events.last();
    evt.set_time(Data->PacketTimestamp());
    evt.set_value(0, channel);

    bool timestamps_included = bool(Data->Timestamps());

    for (size_t i = 0; i < event_count; ++i)
    {
      evt.trace(0)[i] = Data->Values()->Get(i);
      if (timestamps_included)
        evt.trace(1)[i] = Data->Timestamps()->Get(i);
    }
    ++run_spill->events;
    run_spill->events.finalize();
  }
  spill_queue->enqueue(run_spill);
  pushed_spills++;

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

