#include <core/spill.h>
#include <core/util/time_extensions.h>
#include <core/util/ascii_tree.h>
#include <core/util/custom_logger.h>

namespace DAQuiri {

Spill::Spill(std::string id, StatusType t)
    : stream_id(id), type(t)
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

std::string Spill::debug(std::string prepend) const
{
  std::stringstream ss;
  ss << "SPILL [" << type_to_str(type) + "]";
  if (stream_id.size())
    ss << " '" << stream_id << "' ";
  ss << " @ " << boost::posix_time::to_iso_extended_string(time) << "\n";

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
  j["type"] = type_to_str(s.type);
  j["stream_id"] = s.stream_id;
  j["time"] = boost::posix_time::to_iso_extended_string(s.time);
//  j["bytes_raw_data"] = data.size() * sizeof(char);
//  j["number_of_events"] = events.size();
  j["event_model"] = s.event_model;

  if (!s.state.branches.empty())
    j["state"] = s.state;
}

void from_json(const json& j, Spill& s)
{
  s.type = type_from_str(j["type"]);
  s.stream_id = j["stream_id"];
  s.time = from_iso_extended(j["time"].get<std::string>());
  s.event_model = j["event_model"];

  if (j.count("state"))
    s.state = j["state"];
}

}
