/*******************************************************************************
 *
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 *
 * This software can be redistributed and/or modified freely provided that
 * any derivative works bear some notice that they are derived from it, and
 * any modified versions bear some notice that they have been modified.
 *
 * Author(s):
 *      Martin Shetty (NIST)
 *
 ******************************************************************************/

#pragma once

#include "hit.h"
#include <map>

namespace DAQuiri {

struct Event
{
private:
  TimeStamp              lower_time_;
  TimeStamp              upper_time_;
  double                 window_ns_    {0.0};
  double                 max_delay_ns_ {0.0};
  std::multimap<int16_t, Hit> hits_;

public:
  inline Event() {}

  inline Event(const Hit &newhit, double win, double max_delay)
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

}
