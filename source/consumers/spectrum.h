#pragma once

#include "consumer.h"

namespace DAQuiri {

struct Status
{
  static Status extract(const Spill& spill);
  static boost::posix_time::time_duration
  calc_diff(const std::vector<Status>& stats, std::string name);

  StatusType type {StatusType::daq_status};
  boost::posix_time::ptime producer_time;
  boost::posix_time::ptime consumer_time;
  std::map<std::string, Setting> stats;
  TimeBase timebase;
};

enum ClearReferenceTime : int32_t
{
  ProducerWallClock = 0,
  ConsumerWallClock = 1,
  NativeTime = 2
};

struct PeriodicTrigger
{
  bool triggered_{false};

  bool enabled_{false};
  ClearReferenceTime clear_using_{ClearReferenceTime::ProducerWallClock};
  boost::posix_time::time_duration timeout_;

  void settings(const Setting& s);
  Setting settings(int32_t index) const;

  void update_times(const Status& from, const Status& to);
  void eval_trigger();

  boost::posix_time::time_duration recent_native_time_
      {boost::posix_time::seconds(0)};
  boost::posix_time::time_duration recent_producer_wall_time_
      {boost::posix_time::seconds(0)};
  boost::posix_time::time_duration recent_consumer_wall_time_
      {boost::posix_time::seconds(0)};
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
    PeriodicTrigger periodic_trigger_;

    // instantaneous rate:
    PreciseFloat recent_count_{0};
    Status recent_start_, recent_end_;

    std::vector<Status> stats_;
    boost::posix_time::time_duration real_time_;
    boost::posix_time::time_duration live_time_;

    void calc_cumulative();
    void calc_recent_rate(const Spill& spill);

    static Status extract(const Spill& spill);
};

}