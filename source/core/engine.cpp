#include "engine.h"
#include "custom_logger.h"
#include "producer_factory.h"
#include "custom_timer.h"

#include <functional>

#define THREAD_CLOSE_WAIT_TIME_MS 100

namespace DAQuiri {

Engine::Engine()
{
  SettingMeta e0 {"ProfileDescr", SettingType::text, "Profile description"};
  setting_definitions_[e0.id()] = e0;

  SettingMeta e1 {"DropPackets", SettingType::menu, "Drop spills"};
  e1.set_enum(0, "Never");
  e1.set_enum(1, "Stream size limit");
  setting_definitions_[e1.id()] = e1;

  SettingMeta e2 {"MaxPackets", SettingType::integer, "Maximum spills per stream"};
  e2.set_val("min", 1);
  setting_definitions_[e2.id()] = e2;

//  settings_ = default_settings();
}

Setting Engine::default_settings()
{
  Setting ret {SettingMeta("Engine", SettingType::stem)};
  ret.branches.add(Setting::text("ProfileDescr", "(no description)"));
  ret.branches.add(SettingMeta("DropPackets", SettingType::menu));
  ret.branches.add(SettingMeta("MaxPackets", SettingType::integer));
  return ret;
}


void Engine::initialize(const json &profile)
{
  UNIQUE_LOCK_EVENTUALLY_ST

  Setting tree = profile;

  auto& pf = ProducerFactory::singleton();

  _die();
  producers_.clear();
  for (auto &q : tree.branches)
  {
    if (setting_definitions_.count(q.id()))
      continue;
    std::string name = q.get_text();
    ProducerPtr device = pf.create_type(q.id());
    if (!device || name.empty())
    {
      WARN << "<Engine> Failed to load producer"
           << "  type=" << q.id()
           << "  name=\"" << name << "\"";
      continue;
    }
    producers_[name] = device;
  }

  _push_settings(tree);
  _get_all_settings();

  std::string descr = tree.find(Setting("ProfileDescr")).get_text();
  INFO << "<Engine> Initialized profile \"" << descr << "\"";
}

Engine::~Engine()
{
  _die();
}

void Engine::boot()
{
  UNIQUE_LOCK_EVENTUALLY_ST

  for (auto &q : producers_)
    if (q.second)
      q.second->boot();

  _get_all_settings();
}

void Engine::die()
{
  UNIQUE_LOCK_EVENTUALLY_ST
  _die();
}

void Engine::_die()
{
  for (auto &q : producers_)
    if (q.second)
      q.second->die();

  _get_all_settings();
}

ProducerStatus Engine::status() const
{
  SHARED_LOCK_ST
  return aggregate_status_;
}

Setting Engine::pull_settings() const
{
  SHARED_LOCK_ST
  return settings_;
}

void Engine::push_settings(const Setting& newsettings)
{
  UNIQUE_LOCK_EVENTUALLY_ST
  _push_settings(newsettings);
}

void Engine::_push_settings(const Setting& newsettings)
{
  settings_ = newsettings;
  _write_settings_bulk();
}

void Engine::read_settings_bulk()
{
  UNIQUE_LOCK_EVENTUALLY_ST
  _read_settings_bulk();
}

void Engine::_read_settings_bulk()
{
  for (auto &set : settings_.branches)
  {
    if (set.id() == "DropPackets")
    {
      set.enrich(setting_definitions_);
      set.set_number(drop_packets_);
    }
    else if (set.id() == "MaxPackets")
    {
      set.enrich(setting_definitions_);
      set.set_number(max_packets_);
      set.enable_if_flag(drop_packets_, "");
    }
    else if (!setting_definitions_.count(set.id()))
    {
      std::string name = set.get_text();
      if (producers_.count(name))
        producers_[name]->read_settings_bulk(set);
    }
  }
}

void Engine::write_settings_bulk()
{
  UNIQUE_LOCK_EVENTUALLY_ST
  _write_settings_bulk();
}

void Engine::_write_settings_bulk()
{
  for (auto &set : settings_.branches)
  {
    if (set.id() == "DropPackets")
    {
      drop_packets_ = set.get_number();
    }
    else if (set.id() == "MaxPackets")
    {
      max_packets_ = set.get_number();
    }
    else if (!setting_definitions_.count(set.id()))
    {
      std::string name = set.get_text();
      if (producers_.count(name))
        producers_[name]->write_settings_bulk(set);
    }
  }
}

OscilData Engine::oscilloscope()
{
  UNIQUE_LOCK_EVENTUALLY_ST;

  OscilData traces;
//  traces.resize(detectors_.size());

  for (auto &q : producers_)
    if ((q.second != nullptr) && (q.second->status() & ProducerStatus::can_oscil))
    {
      OscilData trc = q.second->oscilloscope();
      for (auto &p : trc)
        traces[p.first] = p.second;
    }
  return traces;
}

bool Engine::daq_start(SpillQueue out_queue)
{
  bool success = false;
  for (auto &q : producers_)
    if ((q.second != nullptr) && (q.second->status() & ProducerStatus::can_run))
    {
      success |= q.second->daq_start(out_queue);
    }
  return success;
}

bool Engine::daq_stop()
{
  bool success = false;
  for (auto &q : producers_)
    if ((q.second != nullptr) && (q.second->status() & ProducerStatus::can_run))
    {
      success |= q.second->daq_stop();
    }
  return success;
}

bool Engine::daq_running() const
{
  bool running = false;
  for (auto &q : producers_)
    if ((q.second != nullptr) && (q.second->status() & ProducerStatus::can_run))
    {
      running |= q.second->daq_running();
    }
  return running;
}

void Engine::set_setting(Setting address, Match flags, bool greedy)
{
  UNIQUE_LOCK_EVENTUALLY_ST
  settings_.set(address, flags, greedy);
  _write_settings_bulk();
  _read_settings_bulk();
}

void Engine::get_all_settings()
{
  UNIQUE_LOCK_EVENTUALLY_ST
  _get_all_settings();
}

void Engine::_get_all_settings()
{
  aggregate_status_ = ProducerStatus(0);
  for (auto &q : producers_)
  {
    q.second->get_all_settings();
    aggregate_status_ = aggregate_status_ | q.second->status();
  }
  _read_settings_bulk();
}

void Engine::acquire(ProjectPtr project, Interruptor &interruptor, uint64_t timeout)
{
  UNIQUE_LOCK_EVENTUALLY_ST

  if (!project)
  {
    WARN << "<Engine> No reference to valid daq project";
    return;
  }

  if (!(aggregate_status_ & ProducerStatus::can_run))
  {
    WARN << "<Engine> No devices exist that can perform acquisition";
    return;
  }

  if (timeout > 0)
    INFO << "<Engine> Starting acquisition scheduled for " << timeout << " seconds";
  else
    INFO << "<Engine> Starting acquisition for indefinite run";

  CustomTimer *anouncement_timer = nullptr;
  double secs_between_anouncements = 5;

  SpillMultiqueue parsed_queue(drop_packets_, max_packets_);

  auto builder = std::thread(&Engine::builder_naive, this, &parsed_queue, project);

  SpillPtr spill;
  spill = std::make_shared<Spill>();
  _get_all_settings();
  spill->state = settings_;
//  spill->detectors = detectors_;
  parsed_queue.enqueue(spill);

  if (!daq_start(&parsed_queue))
    ERR << "<Engine> Failed to start device daq threads";

  CustomTimer total_timer(timeout, true);
  anouncement_timer = new CustomTimer(true);

  while (daq_running())
  {
    wait_ms(THREAD_CLOSE_WAIT_TIME_MS);
    if (anouncement_timer->s() > secs_between_anouncements)
    {
      if (timeout > 0)
        INFO << "  RUNNING Elapsed: " << total_timer.done()
             << "  ETA: " << total_timer.ETA()
             << "  Dropped spills: " << parsed_queue.dropped_spills()
             << "  Dropped events: " << parsed_queue.dropped_events();
      else
        INFO << "  RUNNING Elapsed: " << total_timer.done()
             << "  Dropped spills: " << parsed_queue.dropped_spills()
             << "  Dropped events: " << parsed_queue.dropped_events();

      delete anouncement_timer;
      anouncement_timer = new CustomTimer(true);
    }
    if (interruptor.load() || (timeout && total_timer.timeout()))
    {
      if (!daq_stop())
        ERR << "<Engine> Failed to stop device daq threads";
    }
  }

  delete anouncement_timer;

  spill = std::make_shared<Spill>();
  _get_all_settings();
  spill->state = settings_;
  parsed_queue.enqueue(spill);

  wait_ms(THREAD_CLOSE_WAIT_TIME_MS);
  while (parsed_queue.size() > 0)
    wait_ms(THREAD_CLOSE_WAIT_TIME_MS);
  parsed_queue.stop();
  wait_ms(THREAD_CLOSE_WAIT_TIME_MS);

  builder.join();
  INFO << "<Engine::acquire> Acquisition finished"
       << "\n   dropped spills: " << parsed_queue.dropped_spills()
       << "\n   dropped events: " << parsed_queue.dropped_events();
}

ListData Engine::acquire_list(Interruptor& interruptor, uint64_t timeout)
{
  UNIQUE_LOCK_EVENTUALLY_ST

  if (!(aggregate_status_ & ProducerStatus::can_run))
  {
    WARN << "<Engine> No devices exist that can perform acquisition";
    return ListData();
  }

  if (timeout > 0)
    INFO << "<Engine> List mode acquisition scheduled for " << timeout << " seconds";
  else
    INFO << "<Engine> List mode acquisition indefinite run";

  SpillPtr spill;
  ListData result;

  CustomTimer *anouncement_timer = nullptr;
  double secs_between_anouncements = 5;

  spill = std::make_shared<Spill>();
  _get_all_settings();
  spill->state = settings_;
//  spill->detectors = detectors_;
  result.push_back(spill);

  SpillMultiqueue parsed_queue(drop_packets_, max_packets_);

  if (!daq_start(&parsed_queue))
    ERR << "<Engine> Failed to start device daq threads";

  CustomTimer total_timer(timeout, true);
  anouncement_timer = new CustomTimer(true);

  while (daq_running())
  {
    wait_ms(THREAD_CLOSE_WAIT_TIME_MS);
    if (anouncement_timer->s() > secs_between_anouncements)
    {
      INFO << "  RUNNING Elapsed: " << total_timer.done()
           << "  ETA: " << total_timer.ETA()
           << "  Dropped spills: " << parsed_queue.dropped_spills()
           << "  Dropped events: " << parsed_queue.dropped_events();
      delete anouncement_timer;
      anouncement_timer = new CustomTimer(true);
    }
    if (interruptor.load() || (timeout && total_timer.timeout()))
    {
      if (!daq_stop())
        ERR << "<Engine> Failed to stop device daq threads";
    }
  }

  delete anouncement_timer;

  spill = std::make_shared<Spill>();
  _get_all_settings();
  spill->state = settings_;
  parsed_queue.enqueue(spill);

  wait_ms(THREAD_CLOSE_WAIT_TIME_MS);

  while (parsed_queue.size() > 0)
    result.push_back(SpillPtr(parsed_queue.dequeue()));

  parsed_queue.stop();

  INFO << "<Engine::acquire_list> Acquisition finished"
       << "\n   dropped spills: " << parsed_queue.dropped_spills()
       << "\n   dropped events: " << parsed_queue.dropped_events();

  return result;
}

//////STUFF BELOW SHOULD NOT BE USED DIRECTLY////////////
//////ASSUME YOU KNOW WHAT YOU'RE DOING WITH THREADS/////

void Engine::builder_naive(SpillQueue data_queue,
                           ProjectPtr project)
{
  double time {0};
  uint64_t presort_events(0), presort_cycles(0);

  SpillPtr spill;
  while (true)
  {
    spill = data_queue->dequeue();
    if (spill != nullptr)
    {
      CustomTimer presort_timer(true);
      presort_cycles++;
      presort_events += spill->events.size();
      project->add_spill(spill);
      time += presort_timer.s();
    }
    else
      break;
  }

  CustomTimer presort_timer(true);
  project->flush();
  time += presort_timer.s();

  DBG << "<Engine::builder_naive> Finished "
      << "\n   total spills=" << presort_cycles
      << "\n   events=" << presort_events
      << "\n   time=" << time
      << "\n   secs/spill="
      << (time / double(presort_cycles))
      << "\n   events/sec="
      << (double(presort_events) / time);
}

}
