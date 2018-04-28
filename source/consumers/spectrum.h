#pragma once

#include "consumer.h"

using namespace DAQuiri;

struct Status
{
    StatusType               type;
    boost::posix_time::ptime producer_time;
    boost::posix_time::ptime consumer_time;
    std::map<std::string, Setting> stats;
    TimeBase timebase;
};


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
    bool clear_next_spill_ {false};
    bool clear_periodically_ {false};
    int64_t clear_reference_timer_ {0};
    double clear_at_ {0};

    // instantaneous rate:
    PreciseFloat recent_count_ {0};
    Status recent_start_, recent_end_;
    double recent_native_time_ {0};
    double recent_producer_wall_time_ {0};
    double recent_consumer_wall_time_ {0};

    std::list<Status> stats_;
    boost::posix_time::time_duration real_time_;
    boost::posix_time::time_duration live_time_;

    void calc_cumulative();
    void calc_recent_rate(const Spill& spill);

    static Status extract(const Spill& spill);
};
