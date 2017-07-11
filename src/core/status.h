#pragma once

#include <boost/date_time.hpp>
#include "event_model.h"
#include "precise_float.h"
#include "time_extensions.h"

namespace DAQuiri {

enum class StatusType { start, running, stop };

inline StatusType type_from_str(std::string type)
{
  if (type == "start")
    return StatusType::start;
  else if (type == "stop")
    return StatusType::stop;
  else
    return StatusType::running;
}

inline std::string type_to_str(StatusType type)
{
  if (type == StatusType::start)
    return "start";
  else if (type == StatusType::stop)
    return "stop";
  else
    return "running";
}


struct Status
{
private:
  StatusType               type_     {StatusType::running};
  int16_t                  channel_  {-1};
  EventModel               event_model_;
  boost::posix_time::ptime time_;     //timestamp at end of spill
  std::map<std::string, PreciseFloat> stats_;
  
public:
  inline void set_type(StatusType t) { type_ = t; }
  inline void set_channel(int16_t c) { channel_ = c; }
  inline void set_model(EventModel hm) { event_model_ = hm; }
  inline void set_time(boost::posix_time::ptime t) { time_ = t; }
  inline void set_value(const std::string& key, const PreciseFloat& val)
  {
    stats_[key] = val;
  }

  inline StatusType type() const { return type_; }
  inline int16_t channel() const { return channel_; }
  inline EventModel  event_model() const { return event_model_; }
  inline boost::posix_time::ptime time() const { return time_; }
  inline std::map<std::string, PreciseFloat> stats() const { return stats_; }

  inline std::string debug() const
  {
    std::stringstream ss;
    ss << "Status::" << type_to_str(type_) << "("
       << "ch" << channel_
       << "@" << boost::posix_time::to_iso_extended_string(time_)
       << "," << event_model_.debug() << ")";
    for (auto s : stats_)
      ss << " " << s.first << "=" << s.second;
    return ss.str();
  }

  friend void to_json(json& j, const Status& s)
  {
    j["type"] = type_to_str(s.type_);
    j["channel"] = s.channel_;
    j["time_"] = boost::posix_time::to_iso_extended_string(s.time_);
    j["event_model"] = s.event_model_;
    for (auto &i : s.stats_)
      j["stats_"][i.first] = i.second;
  }

  friend void from_json(const json& j, Status& s)
  {
    s.type_ = type_from_str(j["type"]);
    s.channel_ = j["channel"];
    s.time_ = from_iso_extended(j["time_"]);
    s.event_model_ = j["event_model"];

    if (j.count("stats_"))
    {
      auto o = j["stats_"];
      for (json::iterator it = o.begin(); it != o.end(); ++it)
        s.stats_[it.key()] = it.value();
    }
  }

};

}
