#include <core/spill.h>
#include <core/util/time_extensions.h>
#include <core/util/ascii_tree.h>
#include <core/util/logger.h>

namespace DAQuiri
{

Spill::Type Spill::from_str(const std::string& type)
{
  if (type == "start")
    return Spill::Type::start;
  else if (type == "stop")
    return Spill::Type::stop;
  else if (type == "running")
    return Spill::Type::running;
  else
    return Spill::Type::daq_status;
}

std::string Spill::to_str(const Spill::Type& type)
{
  if (type == Spill::Type::start)
    return "start";
  else if (type == Spill::Type::stop)
    return "stop";
  else if (type == Spill::Type::running)
    return "running";
  else
    return "daq_status";
}

Spill::Spill(std::string id, Spill::Type t)
    : stream_id(id), type(t)
{
  if (t != Spill::Type::daq_status)
    state = Setting::stem("stats");
}

bool Spill::empty()
{
  return (raw.empty() && events.empty() && !state);
}

std::string Spill::debug(std::string prepend) const
{
  std::stringstream ss;
  ss << "SPILL [" << to_str(type) + "]";
  if (stream_id.size())
    ss << " '" << stream_id << "' ";
  ss << " @ " << to_iso_extended(time) << "\n";

  ss << prepend << k_branch_mid_B << "event model: "
     << event_model.debug() << "\n";

  if (events.size())
    ss << prepend << k_branch_mid_B << "event_count=" << events.size() << "\n";
  if (raw.size())
    ss << prepend << k_branch_mid_B << "raw_size=" << (raw.size() * sizeof(char)) << "\n";

  ss << prepend << k_branch_end_B
     << state.debug(prepend + "  ", false);

  return ss.str();
}

void to_json(json& j, const Spill& s)
{
  j["type"] = Spill::to_str(s.type);
  j["stream_id"] = s.stream_id;
  j["time"] = to_iso_extended(s.time);
//  j["bytes_raw_data"] = data.size() * sizeof(char);
//  j["number_of_events"] = events.size();
  j["event_model"] = s.event_model;

  if (s.state)
    j["state"] = s.state;
}

void from_json(const json& j, Spill& s)
{
  s.type = Spill::from_str(j["type"]);
  s.stream_id = j["stream_id"];
  s.time = from_iso_extended(j["time"].get<std::string>());
  s.event_model = j["event_model"];

  if (j.count("state"))
    s.state = j["state"];
}

}
