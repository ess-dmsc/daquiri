#pragma once

#include "consumer.h"
#include "coincidence.h"

using namespace DAQuiri;

class Spectrum : public DAQuiri::Consumer
{
public:
  Spectrum();

protected:
  bool _initialize() override;
  void _push_hit(const Event&) override;
  void _push_stats(const Status&) override;
  void _flush() override;

  void _set_detectors(const std::vector<Detector>& dets) override;
  void _recalc_axes() override;

  virtual bool validate_coincidence(const Coincidence&) const;
  virtual void add_coincicence(const Coincidence&) = 0;

protected:
  std::vector<int32_t> cutoff_logic_;
  std::vector<double>  delay_ns_;

  double max_delay_;
  double coinc_window_;

  std::map<int, std::list<Status>> stats_list_;
  std::map<int, boost::posix_time::time_duration> real_times_;
  std::map<int, boost::posix_time::time_duration> live_times_;
  std::vector<int> energy_idx_;

  std::list<Coincidence> backlog;

  uint64_t recent_count_;
  Status recent_start_, recent_end_;

  Pattern pattern_coinc_, pattern_anti_, pattern_add_;
  uint16_t bits_;

  PreciseFloat total_hits_;
  PreciseFloat total_events_;
};
