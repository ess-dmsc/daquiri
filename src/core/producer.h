#pragma once

#include "setting.h"
#include "sync_queue.h"
#include "spill.h"
#include "custom_logger.h"

namespace DAQuiri {

enum ProducerStatus
{
  dead      = 0,
  loaded    = 1 << 0,
  booted    = 1 << 1,
  can_boot  = 1 << 2,
  can_run   = 1 << 3,
  can_oscil = 1 << 4
};

inline ProducerStatus operator|(ProducerStatus a, ProducerStatus b)
  {return static_cast<ProducerStatus>(static_cast<int>(a) | static_cast<int>(b));}
inline ProducerStatus operator&(ProducerStatus a, ProducerStatus b)
  {return static_cast<ProducerStatus>(static_cast<int>(a) & static_cast<int>(b));}
inline ProducerStatus operator^(ProducerStatus a, ProducerStatus b)
  {return static_cast<ProducerStatus>(static_cast<int>(a) ^ static_cast<int>(b));}


using SpillQueue = SynchronizedQueue<Spill*>*;

class Producer
{
public:
  Producer() {}
  virtual ~Producer() {}

  static std::string plugin_name() {return std::string();}
  virtual std::string device_name() const {return std::string();}

  bool load_setting_definitions(const std::string& file);
  void save_setting_definitions(const std::string& file);

  void settings_from_json(const json& j);
  json settings_to_json() const;

  ProducerStatus status() const {return status_;}
  virtual bool boot() {return false;}
  virtual bool die() {status_ = ProducerStatus(0); return true;}

  virtual void write_settings_bulk(Setting &/*set*/) {}
  virtual void read_settings_bulk(Setting &/*set*/) const {}
  virtual void get_all_settings() {}

  virtual std::list<Event> oscilloscope() {return std::list<Event>();}

  virtual bool daq_init() {return true;}
  virtual bool daq_start(SpillQueue) {return false;}
  virtual bool daq_stop() {return true;}
  virtual bool daq_running() {return false;}

protected:
  ProducerStatus                     status_ {ProducerStatus(0)};
  std::map<std::string, SettingMeta> setting_definitions_;
  std::string                        profile_path_;

  Setting get_rich_setting(const std::string& id) const;

private:
  //no copying
  void operator=(Producer const&);
  Producer(const Producer&);

};

typedef std::shared_ptr<Producer> ProducerPtr;

}
