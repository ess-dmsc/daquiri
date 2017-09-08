#pragma once

#include "fb_parser.h"
#include "custom_timer.h"

using namespace DAQuiri;

class GEMHist;
class GEMTrack;

class mo01_nmx : public fb_parser
{
  public:
    mo01_nmx();
    ~mo01_nmx() {}

    std::string plugin_name() const override {return "mo01_nmx";}

    void write_settings_bulk(const Setting&) override;
    void read_settings_bulk(Setting&) const override;

    Spill* process_payload(void*, int16_t chan, TimeBase tb,
                           PayloadStats& stats) override;

  private:
    // cached params


    std::string debug(const GEMHist&);
    std::string debug(const GEMTrack&);
};
