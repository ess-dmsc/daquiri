#pragma once

#include "fb_parser.h"
#include "f142_logdata_generated.h"
#include <map>


using namespace DAQuiri;

class ChopperTDC : public fb_parser
{
public:
  ChopperTDC();
  ~ChopperTDC() {}
  
  std::string plugin_name() const override {return "ChopperTDC";}
  
  void write_settings_bulk(const Setting&) override;
  void read_settings_bulk(Setting&) const override;
  
  uint64_t process_payload(SpillQueue spill_queue, void* msg) override;
  uint64_t stop(SpillQueue spill_queue) override;
  
private:
  // cached params
  
  std::string stream_id_ {"ChopperTDCTimeStamp"};
  std::string SettingsPrefix_ {"chopper"};
  
  std::map<std::string, int> PVNameMap;
  
  EventModel event_model_;
  
  bool started_ {false};
  
  static std::string debug(const LogData& TDCTimeStamp);
};

