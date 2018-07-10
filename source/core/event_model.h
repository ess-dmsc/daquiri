#pragma once

#include <vector>
#include <map>
#include <core/time_stamp.h>

//might want to encapsulate member vars?

namespace DAQuiri {

struct EventModel
{
public:
  TimeBase                      timebase {1,1};

  std::vector<uint32_t>         values;
  std::vector<uint32_t>         maximum;
  std::vector<std::string>      value_names;
  std::map<std::string, size_t> name_to_val;

  std::vector<std::vector<size_t>> traces;
  std::vector<std::string>         trace_names;
  std::map<std::string, size_t>    name_to_trace;

  inline void add_value(const std::string& name, uint32_t max)
  {
    maximum.push_back(max);
    values.push_back(0);
    value_names.push_back(name);
    name_to_val[name] = values.size() - 1;
  }

  inline void add_trace(const std::string& name,
                        std::vector<size_t> dims)
  {
    traces.push_back(dims);
    trace_names.push_back(name);
    name_to_trace[name] = traces.size() - 1;
  }

  inline std::string debug() const
  {
    std::stringstream ss;
    auto tb = timebase.debug();
    if (!tb.empty())
      ss << "timebase=" << tb << " ";
    if (name_to_val.size())
    {
      ss << "VALS ";
      for (auto& n : name_to_val)
        ss << n.first << "(<=" << int(maximum[n.second]) << ") ";
    }
    if (name_to_trace.size())
    {
      ss << "TRACES ";
      for (auto& n : name_to_trace)
      {
        ss << n.first << "( ";
        for (auto t : traces[n.second])
          ss << t << " ";
        ss << ") ";
      }
    }
    return ss.str();
  }

};

inline void to_json(json& j, const EventModel& t)
{
  j["timebase"] = t.timebase;
  for (size_t i=0; i < t.values.size(); ++i)
  {
    json jj;
    jj["name"] = t.value_names[i];
    jj["max"] = t.maximum[i];
    j["values"].push_back(jj);
  }
  for (size_t i=0; i < t.traces.size(); ++i)
  {
    json jj;
    jj["name"] = t.trace_names[i];
    jj["dims"] = t.traces[i];
    j["traces"].push_back(jj);
  }
}

inline void from_json(const json& j, EventModel& t)
{
  t.timebase = j["timebase"];
  if (j.count("values"))
    for (auto it : j["values"])
      t.add_value(it["name"], it["max"].get<uint32_t>());
  if (j.count("traces"))
    for (auto it : j["traces"])
    {
      auto nm = it["name"];
      std::vector<size_t> dims;
      for (auto d : it["dims"])
        dims.push_back(d.get<size_t>());
      t.add_trace(nm, dims);
    }
}


}
