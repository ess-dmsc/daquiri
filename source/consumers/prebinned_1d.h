#pragma once

#include "spectrum.h"

namespace DAQuiri {

class Prebinned1D : public Spectrum
{
  public:
    Prebinned1D();
    Prebinned1D* clone() const override { return new Prebinned1D(*this); }

  protected:
    std::string my_type() const override { return "Prebinned 1D"; }

    void _apply_attributes() override;
    void _init_from_file() override;
    void _recalc_axes() override;

    //event processing
    void _push_event(const Event& event) override;
    void _push_stats_pre(const Spill& spill) override;
    bool _accept_spill(const Spill& spill) override;
    bool _accept_events(const Spill& spill) override;

    // cached parameters:
    std::string trace_name_;

    //from status manifest
    int trace_idx_{-1};

    std::vector<double> domain_;

    //reserve memory
    Entry entry_{{0}, 0};
};

}
