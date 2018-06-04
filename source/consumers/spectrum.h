#pragma once

#include "consumer.h"
#include "periodic_trigger.h"

namespace DAQuiri {

class Spectrum : public Consumer
{
  public:
    Spectrum();

  protected:
    void _apply_attributes() override;
    bool _accept_spill(const Spill& spill) override;
    void _push_stats_pre(const Spill& spill) override;
    void _push_stats_post(const Spill& spill) override;
    void _flush() override;

  protected:
    PeriodicTrigger periodic_trigger_;

    // instantaneous rate:
    PreciseFloat recent_count_{0};

    std::vector<Status> stats_;
    boost::posix_time::time_duration real_time_;
    boost::posix_time::time_duration live_time_;

    std::vector<Status> recent_2_stats_;

    void calc_cumulative();
    void calc_recent_rate(const Spill& spill);
};

}