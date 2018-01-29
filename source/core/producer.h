#pragma once

#include "setting.h"
#include "spill_dequeue.h"
#include "spill.h"
#include "custom_logger.h"

namespace DAQuiri {

class Producer;
using ProducerPtr = std::shared_ptr<Producer>;
using SpillQueue = SpillMultiqueue*;
using OscilData = std::map<std::string, Event>;

enum ProducerStatus
{
  dead      = 0,
  loaded    = 1 << 0,
  booted    = 1 << 1,
  running   = 1 << 2,
  can_boot  = 1 << 3,
  can_run   = 1 << 4,
  can_oscil = 1 << 5
};

ProducerStatus operator|(ProducerStatus a, ProducerStatus b);
ProducerStatus operator&(ProducerStatus a, ProducerStatus b);
ProducerStatus operator^(ProducerStatus a, ProducerStatus b);

class Producer
{
public:
  Producer() {}
  virtual ~Producer() {}

  virtual std::string plugin_name() const = 0;

  ProducerStatus status() const {return status_;}
  json setting_definitions() const;

  virtual void initialize(const json& definitions);
  virtual void boot() = 0;
  virtual void die() = 0;

  virtual void write_settings_bulk(const Setting &/*set*/) {}
  virtual void read_settings_bulk(Setting &/*set*/) const {}
  virtual void get_all_settings() {}

  virtual StreamManifest stream_manifest() const { return StreamManifest(); }

  virtual OscilData oscilloscope() { return OscilData(); }

  virtual bool daq_init() {return true;}
  virtual bool daq_start(SpillQueue) {return false;}
  virtual bool daq_stop() {return true;}
  virtual bool daq_running() {return false;}

protected:
  ProducerStatus                     status_ {ProducerStatus::dead};
  std::map<std::string, SettingMeta> setting_definitions_;

  Setting enrich_and_toggle_presets(Setting) const;
  Setting get_rich_setting(const std::string& id) const;
  void add_definition(const SettingMeta& sm);

private:
  //no copying
  void operator=(Producer const&);
  Producer(const Producer&);
};

}
