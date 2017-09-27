#pragma once

#include "consumer.h"

using namespace DAQuiri;

class Spectrum : public Consumer
{
public:
  Spectrum();

protected:
  bool _initialize() override;
  void _push_stats_pre(const Status& status) override;
  void _push_stats_post(const Status& status) override;
  void _flush() override;

  virtual bool channel_relevant(int16_t) const = 0;

protected:
  bool clear_next_spill_ {false};
  bool clear_periodically_ {false};
  double clear_at_ {0};
  double recent_total_time_ {0};

  // cached results:
  PreciseFloat total_count_ {0};

  // instantaneous rate:
  PreciseFloat recent_count_ {0};
  Status recent_start_, recent_end_;

  struct stats_info_t
  {
      std::list<Status> stats;
      boost::posix_time::time_duration real;
      boost::posix_time::time_duration live;
  };
  std::map<int, stats_info_t> chan_stats;

  static bool value_relevant(int16_t channel, const std::vector<int>& idx);

  PreciseFloat calc_chan_times(stats_info_t& chan);
  PreciseFloat calc_recent_rate(const Status& status);
};
