#pragma once

#include "spectrum.h"
#include "value_filter.h"

class TimeDomain : public Spectrum
{
  public:
    TimeDomain();
    TimeDomain *clone() const override { return new TimeDomain(*this); }

  protected:
    std::string my_type() const override { return "Time-Activity 1D"; }

    void _apply_attributes() override;
    void _init_from_file() override;
    void _recalc_axes() override;

    //event processing
    void _push_event(const Event &event) override;
    void _push_stats_pre(const Spill &spill) override;

    bool _accept_spill(const Spill &spill) override;
    bool _accept_events(const Spill &spill) override;

    //cached parameters
    double time_resolution_{1};
    std::string units_name_;
    double units_multiplier_{1};
    FilterBlock filters_;
    double window_{0.0};
    bool trim_{true};

    double earliest_{0.0};
    std::vector<double> domain_;
    std::vector<double> range_;

    //from status manifest
    TimeBase timebase_;

    //reserve memory
    Entry entry_{{0}, 0};
};
