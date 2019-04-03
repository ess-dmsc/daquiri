#include <consumers/add_ons/status.h>
#include <core/util/logger.h>

namespace DAQuiri {

Status Status::extract(const Spill& spill)
{
  Status ret;
  ret.valid = true;
  ret.type = spill.type;
  ret.producer_time = spill.time;
  ret.consumer_time = std::chrono::system_clock::now();
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
hr_duration_t Status::calc_diff(const Status& from, const Status& to, std::string name)
{
  if (!from.stats.count(name) || !to.stats.count(name))
    return hr_duration_t();
  auto diff = to.stats.at(name).get_number() - from.stats.at(name).get_number();
  auto diff_us = to.timebase.to_microsec(diff);
  return std::chrono::microseconds(static_cast<uint64_t>(diff_us));
}

hr_duration_t Status::total_elapsed(const std::vector<Status>& stats, std::string name)
{
  hr_duration_t t {std::chrono::seconds(0)};
  hr_duration_t ret {std::chrono::seconds(0)};

  Status start = stats.front();

  for (const auto& q : stats)
  {
    if (q.type == Spill::Type::start)
    {
      ret += t;
      start = q;
    }
    else
    {
      t = calc_diff(start, q, name);
      if (t == hr_duration_t())
        return t;
    }
  }

  if (stats.back().type != Spill::Type::start)
    ret += t;

  return ret;
}

}