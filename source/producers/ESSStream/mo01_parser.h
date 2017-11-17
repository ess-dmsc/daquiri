#pragma once

#include "fb_parser.h"
#include "mo01_nmx_generated.h"


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
    SpillPtr dummy_spill(uint64_t utime) override;
    SpillPtr process_payload(void*, uint64_t utime) override;

  private:
    // cached params

    int16_t hists_channel_ {0};
    int16_t trace_x_channel_ {1};
    int16_t trace_y_channel_ {2};

    EventModel hists_model_;
    EventModel trace_model_;

    SpillPtr produce_hists(const GEMHist&, uint64_t utime);
    SpillPtr produce_tracks(const GEMTrack&, uint64_t utime);

    static void grab_hist(Event& e, size_t idx, const flatbuffers::Vector<uint32_t>* data);
    void grab_track(const flatbuffers::Vector<flatbuffers::Offset<pos>>* data,
                    uint64_t utime, int16_t chan, SpillPtr ret);

    static std::string debug(const GEMHist&);
    static std::string debug(const GEMTrack&);

    static std::string print_hist(const flatbuffers::Vector<uint32_t>* data);
    static std::string print_track(const flatbuffers::Vector<flatbuffers::Offset<pos>>* data);
};
