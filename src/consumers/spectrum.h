#pragma once

#include "consumer.h"

using namespace DAQuiri;

class Spectrum : virtual public DAQuiri::Consumer
{
public:
  Spectrum();

protected:
  void _init_from_file(std::string name) override;
  void _recalc_axes() override;

  void _push_stats(const Status&) override;
  void _flush() override;

  virtual bool channel_relevant(int16_t) const = 0;

protected:
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
