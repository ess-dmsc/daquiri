#pragma once

#include "hit.h"
#include "pattern.h"
#include <map>

namespace DAQuiri {

struct Coincidence
{
private:
  TimeStamp              lower_time_;
  TimeStamp              upper_time_;
  double                 window_ns_    {0.0};
  double                 max_delay_ns_ {0.0};
  std::multimap<int16_t, Hit> hits_;

public:
  inline Coincidence() {}

  inline Coincidence(const Hit &newhit, double win, double max_delay)
  {
    upper_time_ = lower_time_ = newhit.timestamp();
    hits_.insert({newhit.source_channel(), newhit});
    window_ns_ = win;
    max_delay_ns_ = std::max(win, max_delay);
  }

  inline bool add_hit(const Hit &newhit)
  {
    bool collision = hits_.count(newhit.source_channel());
    lower_time_ = std::min(lower_time_, newhit.timestamp());
    upper_time_ = std::max(upper_time_, newhit.timestamp());
    hits_.insert({newhit.source_channel(), newhit});
    return collision;
  }

  inline bool empty() const
  {
    return hits_.empty();
  }

  inline size_t size() const
  {
    return hits_.size();
  }

  inline const std::multimap<int16_t, Hit>& hits() const
  {
    return hits_;
  }

  inline TimeStamp lower_time() const
  {
    return lower_time_;
  }

  inline TimeStamp upper_time() const
  {
    return upper_time_;
  }

  inline bool antecedent(const Hit& h) const
  {
    return (h.timestamp() < lower_time_);
  }

  inline bool in_window(const Hit& h) const
  {
    return (h.timestamp() >= lower_time_) &&
        ((h.timestamp() - lower_time_) <= window_ns_);
  }

  inline bool past_due(const Hit& h) const
  {
    return (h.timestamp() >= lower_time_) &&
        ((h.timestamp() - lower_time_) > max_delay_ns_);
  }

  inline std::string debug() const
  {
    std::stringstream ss;
    ss << "Evt[t" << lower_time_.debug()
       << "w" << window_ns_;
    if (max_delay_ns_ != window_ns_)
      ss << "d" << max_delay_ns_;
    ss << "]";
    for (auto &q : hits_)
      ss << " " << q.first << "=" << q.second.debug();
    return ss.str();
  }
};

inline bool validate(const Coincidence &e, const Pattern& p)
{
  if (p.threshold() == 0)
    return true;
  size_t matches = 0;
  for (auto h : e.hits())
  {
    if ((h.first < 0) ||
        (h.first >= int16_t(p.gates().size())))
      continue;
    else if (p.gates()[h.first])
      matches++;
    if (matches == p.threshold())
      break;
  }
  return (matches == p.threshold());
}

inline bool antivalidate(const Coincidence &e, const Pattern& p)
{
  if (p.threshold() == 0)
    return true;
  size_t matches = p.threshold();
  for (auto h : e.hits())
  {
    if ((h.first < 0) ||
        (h.first >= int16_t(p.gates().size())))
      continue;
    else if (p.gates()[h.first])
      matches--;
    if (matches < p.threshold())
      break;
  }
  return (matches == p.threshold());
}

}
