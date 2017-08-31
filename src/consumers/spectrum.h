#pragma once

#include "consumer.h"

using namespace DAQuiri;

class Spectrum : public Consumer
{
public:
  Spectrum();

protected:
  bool _initialize() override;
  void _push_spill(const Spill&spill) override;
  void _push_stats(const Status& status) override;
  void _flush() override;

  virtual bool channel_relevant(int16_t) const = 0;

protected:
  bool clear_next_spill_ {false};
  uint64_t clear_at_ {0};
  double recent_total_time_ {0};

  // cached results:
  PreciseFloat total_count_ {0};

  // instantaneous rate:
  PreciseFloat recent_count_ {0};
  std::map<int, std::list<Status>> stats_list_;
  std::map<int, boost::posix_time::time_duration> real_times_;
  std::map<int, boost::posix_time::time_duration> live_times_;
  Status recent_start_, recent_end_;

  static bool value_relevant(int16_t channel, const std::vector<int>& idx);
};
