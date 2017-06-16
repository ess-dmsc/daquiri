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

#include "stats_update.h"
#include "util.h"

namespace DAQuiri {

StatsUpdate::Type StatsUpdate::type_from_str(std::string type)
{
  if (type == "start")
    return Type::start;
  else if (type == "stop")
    return Type::stop;
  else
    return Type::running;
}

std::string StatsUpdate::type_to_str(StatsUpdate::Type type)
{
  if (type == Type::start)
    return "start";
  else if (type == Type::stop)
    return "stop";
  else
    return "running";
}

// difference across all variables
// except rate wouldn't make sense
// and timestamp would require duration output type
StatsUpdate StatsUpdate::operator-(const StatsUpdate other) const
{
  StatsUpdate answer;
  if (source_channel != other.source_channel)
    return answer;

  //labtime?
  for (auto &i : items)
    if (other.items.count(i.first))
      answer.items[i.first] = i.second - other.items.at(i.first);
  return answer;
}

bool StatsUpdate::operator==(const StatsUpdate other) const
{
  if (stats_type != other.stats_type)
    return false;
  if (source_channel != other.source_channel)
    return false;
  if (items != other.items)
    return false;
  return true;
}

// stacks two, adding up all variables
// except timestamp wouldn't make sense
StatsUpdate StatsUpdate::operator+(const StatsUpdate other) const
{
  StatsUpdate answer;
  if (source_channel != other.source_channel)
    return answer;

  //labtime?
  for (auto &i : items)
    if (other.items.count(i.first))
      answer.items[i.first] = i.second + other.items.at(i.first);
  return answer;
}

std::string StatsUpdate::to_string() const
{
  std::stringstream ss;
  ss << "Stats::" << type_to_str(stats_type) << "("
     << "ch" << source_channel
     << "@" << boost::posix_time::to_iso_extended_string(lab_time)
     << ")";
  return ss.str();
}

void to_json(json& j, const StatsUpdate& s)
{
  j["type"] = StatsUpdate::type_to_str(s.stats_type);
  j["channel"] = s.source_channel;
  j["lab_time"] = boost::posix_time::to_iso_extended_string(s.lab_time);
  j["hit_model"] = s.model_hit;
  for (auto &i : s.items)
    j["items"][i.first] = i.second;
}

void from_json(const json& j, StatsUpdate& s)
{
  s.stats_type = StatsUpdate::type_from_str(j["type"]);
  s.source_channel = j["channel"];
  s.lab_time = from_iso_extended(j["lab_time"]);
  s.model_hit = j["hit_model"];

  if (j.count("items"))
  {
    auto o = j["items"];
    for (json::iterator it = o.begin(); it != o.end(); ++it)
      s.items[it.key()] = it.value();
  }
}



}
