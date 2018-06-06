#include "status.h"
#include "custom_logger.h"

namespace DAQuiri {

Status Status::extract(const Spill& spill)
{
  Status ret;
  ret.valid = true;
  ret.type = spill.type;
  ret.producer_time = spill.time;
  ret.consumer_time = boost::posix_time::microsec_clock::universal_time();
  ret.timebase = spill.event_model.timebase;
  Setting native_time = spill.state.find(Setting("native_time"));
  if (native_time)
    ret.stats["native_time"] = native_time;
  auto live_time = spill.state.find(Setting("live_time"));
  if (live_time)
    ret.stats["live_time"] = live_time;
  return ret;
}

// TODO: make this work with nanoseconds
boost::posix_time::time_duration
Status::calc_diff(const Status& from, const Status& to, std::string name)
{
  if (!from.stats.count(name) || !to.stats.count(name))
    return boost::posix_time::not_a_date_time;
  auto diff = to.stats.at(name).get_number() - from.stats.at(name).get_number();
  auto diff_us = to.timebase.to_microsec(diff);
  return boost::posix_time::microseconds(static_cast<uint64_t>(diff_us));
}

boost::posix_time::time_duration
Status::total_elapsed(const std::vector<Status>& stats, std::string name)
{
  boost::posix_time::time_duration t {boost::posix_time::seconds(0)};
  boost::posix_time::time_duration ret {boost::posix_time::seconds(0)};

  Status start = stats.front();

  for (const auto& q : stats)
  {
    if (q.type == StatusType::start)
    {
      ret += t;
      start = q;
    }
    else
    {
      t = calc_diff(start, q, name);
      if (t.is_not_a_date_time())
        return t;
    }
  }

  if (stats.back().type != StatusType::start)
    ret += t;

  return ret;
}

}