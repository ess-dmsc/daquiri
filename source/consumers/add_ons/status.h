#pragma once

#include <core/spill.h>

namespace DAQuiri {

struct Status {
  static Status extract(const Spill &spill);

  static hr_duration_t calc_diff(const Status &from, const Status &to,
                                 std::string name);

  static hr_duration_t total_elapsed(const std::vector<Status> &stats,
                                     std::string name);

  bool valid{false};
  Spill::Type type{Spill::Type::daq_status};
  hr_time_t producer_time{};
  hr_time_t consumer_time{};
  std::map<std::string, Setting> stats;
  TimeBase timebase;
};

} // namespace DAQuiri