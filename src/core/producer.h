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
using OscilData = std::vector<Event>;

class Producer
{
public:
  Producer() {}
  virtual ~Producer() {}

  static std::string plugin_name() {return std::string();}
  ProducerStatus status() const {return status_;}
  json setting_definitions() const;

  virtual std::string device_name() const {return std::string();}

  virtual void initialize(const json& definitions);
  virtual void boot() = 0;
  virtual void die() = 0;

  virtual void write_settings_bulk(const Setting &/*set*/) {}
  virtual void read_settings_bulk(Setting &/*set*/) const {}
  virtual void get_all_settings() {}

  virtual OscilData oscilloscope() { return OscilData(); }

  virtual bool daq_init() {return true;}
  virtual bool daq_start(SpillQueue) {return false;}
  virtual bool daq_stop() {return true;}
  virtual bool daq_running() {return false;}

protected:
  ProducerStatus                     status_ {ProducerStatus::dead};
  std::map<std::string, SettingMeta> setting_definitions_;

  Setting get_rich_setting(const std::string& id) const;
  void add_definition(const SettingMeta& sm);

private:
  //no copying
  void operator=(Producer const&);
  Producer(const Producer&);

};

typedef std::shared_ptr<Producer> ProducerPtr;

}
