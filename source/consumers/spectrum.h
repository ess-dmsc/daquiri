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
  void _push_stats_pre(const Spill& spill) override;
  void _push_stats_post(const Spill& spill) override;
  void _flush() override;

protected:
  bool clear_next_spill_ {false};
  bool clear_periodically_ {false};
  double clear_at_ {0};
  double recent_total_time_ {0};

  // cached results:
  PreciseFloat total_count_ {0};

  // instantaneous rate:
  PreciseFloat recent_count_ {0};
  Spill recent_start_, recent_end_;

  struct stats_info_t
  {
      std::list<Status> stats;
      boost::posix_time::time_duration real;
      boost::posix_time::time_duration live;
  };
  stats_info_t chan_stats;

  PreciseFloat calc_chan_times();
  PreciseFloat calc_recent_rate(const Spill& spill);

  static Status extract(const Spill& spill);
};
