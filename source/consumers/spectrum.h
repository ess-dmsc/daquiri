#pragma once

#include "consumer.h"

using namespace DAQuiri;

struct Status
{
    StatusType               type;
    boost::posix_time::ptime time;
    std::map<std::string, Setting> stats;
    TimeBase tb;
};


class Spectrum : public Consumer
{
  public:
    Spectrum();

  protected:
    bool _initialize() override;
    bool _accept_spill(const Spill& spill) override;
    void _push_stats_pre(const Spill& spill) override;
    void _push_stats_post(const Spill& spill) override;
    void _flush() override;

  protected:
    bool clear_next_spill_ {false};
    bool clear_periodically_ {false};
    double clear_at_ {0};

    // cached results:
    PreciseFloat total_count_ {0};

    // instantaneous rate:
    PreciseFloat recent_count_ {0};
    double recent_total_time_ {0};
    Status recent_start_, recent_end_;

    std::list<Status> stats_;
    boost::posix_time::time_duration real_time_;
    boost::posix_time::time_duration live_time_;

    void calc_cumulative();
    PreciseFloat calc_recent_rate(const Spill& spill);

    static Status extract(const Spill& spill);
};
