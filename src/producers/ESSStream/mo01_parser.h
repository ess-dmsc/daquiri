#pragma once

#include "fb_parser.h"

using namespace DAQuiri;

class GEMHist;
class GEMTrack;
class MonitorMessage;

class mo01_nmx : public fb_parser
{
  public:
    mo01_nmx();
    ~mo01_nmx() {}

    std::string plugin_name() const override {return "mo01_nmx";}

    void write_settings_bulk(const Setting&) override;
    void read_settings_bulk(Setting&) const override;

    SpillPtr start_spill() const override;
    SpillPtr stop_spill() const override;
    SpillPtr process_payload(void*, TimeBase tb,
                           uint64_t utime,
                           PayloadStats& stats) override;
    SpillPtr dummy_spill(uint64_t utime,
                         PayloadStats& stats) override;

  private:
    // cached params

    int16_t hists_channel_ {0};
    int16_t trace_x_channel_ {1};
    int16_t trace_y_channel_ {2};

    EventModel hists_model_;
    EventModel trace_model_;

    bool is_empty(const MonitorMessage *);

    void produce_hists(const GEMHist&, uint64_t utime, SpillPtr);
    void produce_tracks(const GEMTrack&, uint64_t utime, SpillPtr);

    std::string debug(const GEMHist&);
    std::string debug(const GEMTrack&);
};
