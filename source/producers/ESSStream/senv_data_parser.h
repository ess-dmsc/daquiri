#pragma once

#include "fb_parser.h"
#include "senv_data_generated.h"
#include <map>

using namespace DAQuiri;

class SenvParser : public fb_parser
{
  public:
    SenvParser();

    ~SenvParser() {}

    std::string plugin_name() const override { return "SenvParser"; }

    void settings(const Setting&) override;
    Setting settings() const override;

    uint64_t process_payload(SpillQueue spill_queue, void* msg) override;
    uint64_t stop(SpillQueue spill_queue) override;

    StreamManifest stream_manifest() const override;

  private:
    // cached params

    std::string stream_id_{"SenvParser"};

    std::map<std::string, int> PVNameMap;

    EventModel event_model_;

    bool started_{false};

//    static std::string debug(const LogData& TDCTimeStamp);
};

