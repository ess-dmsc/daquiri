#pragma once

#include <producers/ESSStream/fb_parser.h>
#include "f142_logdata_generated.h"
#include <map>

using namespace DAQuiri;

class ChopperTDC : public fb_parser
{
 public:
  ChopperTDC();

  ~ChopperTDC() = default;

  std::string plugin_name() const override
  { return "ChopperTDC"; }

  std::string schema_id() const override;
  std::string get_source_name(void* msg) const override;

  void settings(const Setting&) override;
  Setting settings() const override;

  uint64_t process_payload(SpillMultiqueue * spill_queue, void* msg) override;
  uint64_t stop(SpillMultiqueue * spill_queue) override;

  StreamManifest stream_manifest() const override;

 private:
  // cached params

  std::string stream_id_{"ChopperTDC"};

  bool filter_source_name_{false};
  std::string source_name_;

  EventModel event_model_;

  bool started_{false};

  static std::string debug(const LogData& TDCTimeStamp);
};
