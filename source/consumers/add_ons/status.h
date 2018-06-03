#pragma once

#include "spill.h"

namespace DAQuiri {

struct Status
{
  static Status extract(const Spill& spill);

  static boost::posix_time::time_duration
  calc_diff(const Status& from, const Status& to, std::string name);

  static boost::posix_time::time_duration
  total_elapsed(const std::vector<Status>& stats, std::string name);

  bool valid {false};
  StatusType type{StatusType::daq_status};
  boost::posix_time::ptime producer_time;
  boost::posix_time::ptime consumer_time;
  std::map<std::string, Setting> stats;
  TimeBase timebase;
};

}