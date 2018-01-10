#pragma once

#include "fb_parser.h"
#include "MonitorPeakData_generated.h"


using namespace DAQuiri;

class Monitor : public fb_parser
{
  public:
    Monitor();
    ~Monitor() {}

    std::string plugin_name() const override {return "Monitor";}

    void write_settings_bulk(const Setting&) override;
    void read_settings_bulk(Setting&) const override;

    uint64_t process_payload(SpillQueue spill_queue, void* msg) override;
    uint64_t stop(SpillQueue spill_queue) override;

  private:
    // cached params

    std::string stream_id_ {"AdcMonitorData"};
    std::string SettingsPrefix_ {"Monitor"};

    EventModel event_model_;

    bool started_ {false};
  
    static std::string debug(const MonitorPeakData& Event);
};
