#pragma once

#include <producers/ESSStream/fb_parser.h>
#include <map>

using namespace DAQuiri;

class SampleEnvironmentData;

class SenvParser : public fb_parser
{
 public:
  SenvParser();

  ~SenvParser()
  {}

  std::string plugin_name() const override
  { return "SenvParser"; }

  std::string schema_id() const override;
  std::string get_source_name(void* msg) const override;

  void settings(const Setting&) override;
  Setting settings() const override;

  uint64_t process_payload(SpillQueue spill_queue, void* msg) override;
  uint64_t stop(SpillQueue spill_queue) override;

  StreamManifest stream_manifest() const override;

 private:
  // cached params

  std::string stream_id_base_{"Senv"};

  bool filter_source_name_{false};
  std::string source_name_;

  EventModel event_model_;

  bool started_{false};

  static std::string debug(const SampleEnvironmentData* TDCTimeStamp);

  uint64_t start(SpillQueue spill_queue);

};

