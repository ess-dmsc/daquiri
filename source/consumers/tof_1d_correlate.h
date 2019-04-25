#pragma once

#include <consumers/spectrum.h>

namespace DAQuiri {

class TOF1DCorrelate : public Spectrum
{
  public:
    TOF1DCorrelate();
    TOF1DCorrelate* clone() const override { return new TOF1DCorrelate(*this); }

  protected:
    std::string my_type() const override { return "Time of Flight 1D (w/ stream correlation)"; }

    void _apply_attributes() override;
    void _init_from_file() override;
    void _recalc_axes() override;

    //event processing
    void _push_stats_pre(const Spill& spill) override;
    void _push_event(const Event& event) override;
    void _push_stats_post(const Spill& spill) override;

    bool _accept_spill(const Spill& spill) override;
    bool _accept_events(const Spill& spill) override;

    // cached parameters:
    double time_resolution_{1};
    std::string units_name_;
    double units_multiplier_{1};

    std::string chopper_stream_id_;

    // from status manifest
    TimeBase timebase_;
    TimeBase chopper_timebase_;

    std::deque<uint64_t> events_buffer_;
    std::deque<uint64_t> chopper_buffer_;

    std::vector<double> domain_;

    //reserve memory
    Coords coords_{0};

    bool bin_events();
    bool can_bin() const;
};

}
