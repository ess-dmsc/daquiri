#include "spill.h"
#include "time_extensions.h"

#include "custom_logger.h"

namespace DAQuiri {

Spill::Spill(std::string id, StatusType t)
  : stream_id (id)
  , type (t)
{
  if (t != StatusType::daq_status)
    state = Setting::stem("stats");
}

bool Spill::empty()
{
  return (
          raw.empty() &&
          events.empty() &&
          state.branches.empty()
          );
}

std::string Spill::to_string() const
{
  std::string info = type_to_str(type) + " "
      + stream_id + " " + boost::posix_time::to_iso_extended_string(time);
  if (events.size())
  {
    info += " " + event_model.debug();
    info += " event_count=" + std::to_string(events.size());
  }
  if (raw.size())
    info += " RAW=" + std::to_string(raw.size() * sizeof(char));
  return info;
}

void to_json(json& j, const Spill& s)
{
  j["time"] = boost::posix_time::to_iso_extended_string(s.time);
//  j["bytes_raw_data"] = data.size() * sizeof(char);
//  j["number_of_events"] = events.size();

  if (!s.state.branches.empty())
    j["state"] = s.state;
}

void from_json(const json& j, Spill& s)
{
  s.time = from_iso_extended(j["time"].get<std::string>());

  if (j.count("state"))
    s.state = j["state"];
}


}
