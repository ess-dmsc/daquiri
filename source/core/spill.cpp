#include "spill.h"
#include "time_extensions.h"

#include "custom_logger.h"

namespace DAQuiri {

Spill::Spill(StatusType t)
  : type (t)
{}

//SpillPtr Spill::make_new(StatusType t,
//                         std::initializer_list<int16_t> channels)
//{
//  SpillPtr ret = std::make_shared<Spill>();
//  for (auto c :channels)
//  {
//    ret->stats[c] = Status(c, t);
//    ret->stats[c].set_time(ret->time);
//  }
//  return ret;
//}

bool Spill::empty()
{
  return (
//        stats.empty() &&
//          detectors.empty() &&
          raw.empty() &&
          events.empty() &&
          state.branches.empty()
          );
}

std::string Spill::to_string() const
{
  std::string info = boost::posix_time::to_iso_extended_string(time);
//  if (detectors.size())
//    info += " D" + std::to_string(detectors.size());
//  if (stats.size())
//    info += " S" + std::to_string(stats.size());
  if (events.size())
    info += " [" + std::to_string(events.size()) + "]";
  if (raw.size())
    info += " RAW=" + std::to_string(raw.size() * sizeof(char));
  return info;
}

void to_json(json& j, const Spill& s)
{
  j["time"] = boost::posix_time::to_iso_extended_string(s.time);
//  j["bytes_raw_data"] = data.size() * sizeof(char);
//  j["number_of_events"] = events.size();

//  for (auto &st : s.stats)
//    j["stats"].push_back(st.second);

  if (!s.state.branches.empty())
    j["state"] = s.state;

//  if (!s.detectors.empty())
//    j["detectors"] = s.detectors;
}

void from_json(const json& j, Spill& s)
{
  s.time = from_iso_extended(j["time"].get<std::string>());

//  if (j.count("stats"))
//    for (auto it : j["stats"])
//    {
//      Status st = it;
//      s.stats[st.channel()] = st;
//    }

  if (j.count("state"))
    s.state = j["state"];

//  if (j.count("detectors"))
//    for (auto it : j["detectors"])
//      s.detectors.push_back(it);
}


}
