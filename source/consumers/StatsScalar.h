#pragma once

#include <consumers/Spectrum.h>

namespace DAQuiri {

class StatsScalar : public Spectrum
{
  public:
    StatsScalar();

    StatsScalar* clone() const override { return new StatsScalar(*this); }

  protected:
    std::string my_type() const override { return "Stats Scalar"; }

    void _apply_attributes() override;
    void _recalc_axes() override;

    //event processing
    void _push_event(const Event& event) override;
    void _push_stats_pre(const Spill& spill) override;
    bool _accept_events(const Spill& spill) override;
    void _flush() override;

    // cached parameters:
    std::string what_{0};

    bool diff_{false};
    PreciseFloat previous_{0};

    //reserve memory
    Entry entry_{{}, {0}};
};

}
