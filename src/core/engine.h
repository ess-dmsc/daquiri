#pragma once

#include <boost/thread.hpp>
#include <boost/atomic.hpp>

#include "project.h"
#include "producer.h"
#include "sync_queue.h"

namespace DAQuiri {

using Interruptor = boost::atomic<bool>;

class Engine
{
public:
  static Engine& singleton()
  {
    static Engine singleton_instance;
    return singleton_instance;
  }

  void initialize(const json& profile, const json &definitions);
  void boot();
  void die();

  ProducerStatus status() const;
  std::vector<Event> oscilloscope();
  ListData acquire_list(Interruptor& inturruptor, uint64_t timeout);
  void acquire(ProjectPtr project, Interruptor &interruptor,
               uint64_t timeout);

  /////SETTINGS/////
  Setting pull_settings() const;
  void push_settings(const Setting&);
  void set_setting(Setting address, Match flags, bool greedy = false);
  void write_settings_bulk();
  void read_settings_bulk();
  void get_all_settings();

  //detectors
  std::vector<Detector> get_detectors() const;
  void set_detector(size_t, Detector);
  void load_optimizations();

//  static int print_version();
//  static std::string version();

private:
  mutable boost::shared_mutex shared_mutex_;
  mutable boost::mutex unique_mutex_;

  ProducerStatus aggregate_status_ {ProducerStatus(0)};

  std::map<std::string, ProducerPtr> devices_;

  Setting settings_ {SettingMeta("Engine", SettingType::stem)};
  std::vector<Detector> detectors_;

  void _die();
  void _push_settings(const Setting&);
  void _get_all_settings();
  void _read_settings_bulk();
  void _write_settings_bulk();

  void save_det_settings(Setting&, const Setting&, Match flags) const;
  void load_det_settings(Setting, Setting&, Match flags);
  void rebuild_structure(Setting &set);

  void save_optimization();
  void load_optimization(size_t);

  bool daq_start(SpillQueue out_queue);
  bool daq_stop();
  bool daq_running() const;

  //threads
  void worker_chronological(SpillQueue data_queue,
                            ProjectPtr project);

  //singleton assurance
  Engine();
  Engine(Engine const&);
  void operator=(Engine const&);
  ~Engine();

};

}
