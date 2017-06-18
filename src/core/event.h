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

struct Event {
  TimeStamp              lower_time;
  double                 window_ns    {0.0};
  double                 max_delay_ns {0.0};
  std::map<int16_t, Hit> hits;

  inline Event() {}

  inline Event(const Hit &newhit, double win, double max_delay)
  {
    lower_time = newhit.timestamp();
    hits[newhit.source_channel()] = newhit;
    window_ns = win;
    max_delay_ns = std::max(win, max_delay);
  }

  inline bool antecedent(const Hit& h) const
  {
    return (h.timestamp() < lower_time);
  }

  inline bool in_window(const Hit& h) const
  {
    return (h.timestamp() >= lower_time) &&
        ((h.timestamp() - lower_time) <= window_ns);
  }

  inline bool past_due(const Hit& h) const
  {
    return (h.timestamp() >= lower_time) &&
        ((h.timestamp() - lower_time) > max_delay_ns);
  }
  inline bool add_hit(const Hit &newhit)
  {
    if (hits.count(newhit.source_channel()))
      return false;
    if (lower_time > newhit.timestamp())
      lower_time = newhit.timestamp();
    hits[newhit.source_channel()] = newhit;
    return true;
  }

  inline std::string to_string() const
  {
    std::stringstream ss;
    ss << "EVT[t" << lower_time.debug() << "w" << window_ns << "]";
    for (auto &q : hits)
      ss << " " << q.first << "=" << q.second.debug();
    return ss.str();
  }

};

}
