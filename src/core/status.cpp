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

#include "status.h"
#include "util.h"

namespace DAQuiri {

void to_json(json& j, const Status& s)
{
  j["type"] = type_to_str(s.type_);
  j["channel"] = s.channel_;
  j["time_"] = boost::posix_time::to_iso_extended_string(s.time_);
  j["hit_model"] = s.hit_model_;
  for (auto &i : s.stats_)
    j["stats_"][i.first] = i.second;
}

void from_json(const json& j, Status& s)
{
  s.type_ = type_from_str(j["type"]);
  s.channel_ = j["channel"];
  s.time_ = from_iso_extended(j["time_"]);
  s.hit_model_ = j["hit_model"];

  if (j.count("stats_"))
  {
    auto o = j["stats_"];
    for (json::iterator it = o.begin(); it != o.end(); ++it)
      s.stats_[it.key()] = it.value();
  }
}

}
