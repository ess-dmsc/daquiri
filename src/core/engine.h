#pragma once

#include <boost/thread.hpp>
#include <boost/atomic.hpp>

#include "detector.h"
#include "setting.h"
#include "producer.h"
#include "sync_queue.h"
#include "project.h"

#include "custom_timer.h"

namespace DAQuiri {

class Engine {
  
public:

  static Engine& singleton()
  {
    static Engine singleton_instance;
    return singleton_instance;
  }

  void initialize(std::string profile_path, std::string settings_path);
  bool boot();
  bool die();
  ProducerStatus status() {return aggregate_status_;}

  ListData acquire_list(uint64_t timeout, boost::atomic<bool>& inturruptor);
  void acquire(uint64_t timeout, ProjectPtr spectra,
              boost::atomic<bool> &interruptor);

  //detectors
  std::vector<Detector> get_detectors() const {return detectors_;}
  void set_detector(size_t, Detector);

  void load_optimization();

  void set_setting(Setting address, Match flags, bool greedy);

  /////SETTINGS/////
  Setting pull_settings() const;
  void push_settings(const Setting&);
  bool write_settings_bulk();
  bool read_settings_bulk(); 
  void get_all_settings();

  std::vector<Event> oscilloscope();
  
  bool daq_start(SynchronizedQueue<Spill*>* out_queue);
  bool daq_stop();
  bool daq_running();

//  static int print_version();
//  static std::string version();

protected:
  std::string profile_path_;
  ProducerStatus aggregate_status_ {ProducerStatus(0)};
  ProducerStatus intrinsic_status_ {ProducerStatus(0)};
  mutable boost::mutex mutex_;

  std::map<std::string, ProducerPtr> devices_;

  Setting settings_tree_;
  SettingMeta total_det_num_;

  std::vector<Detector> detectors_;

  void save_det_settings(Setting&, const Setting&, Match flags) const;
  void load_det_settings(Setting, Setting&, Match flags);
  void rebuild_structure(Setting &set);

  //threads
  void worker_chronological(SynchronizedQueue<Spill*>* data_queue,
                            ProjectPtr spectra);

private:

  //singleton assurance
  Engine();
  Engine(Engine const&);
  void operator=(Engine const&);
  ~Engine();

  void save_optimization();
  void load_optimization(size_t);

};

}
