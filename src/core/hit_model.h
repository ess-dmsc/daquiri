#pragma once

#include <vector>
#include <map>
#include "digitized_value.h"
#include "time_stamp.h"

//might want to encapsulate member vars?

namespace DAQuiri {

struct HitModel
{
public:
  TimeBase                      timebase;

  std::vector<DigitizedVal>     values;
  std::vector<std::string>      value_names;
  std::map<std::string, size_t> name_to_val;

  std::vector<std::vector<size_t>> traces;
  std::vector<std::string>         trace_names;
  std::map<std::string, size_t>    name_to_trace;

  inline void add_value(const std::string& name, uint16_t bits)
  {
    values.push_back(DigitizedVal(0,bits));
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
    for (auto &n : name_to_val)
      ss << n.first << "(" << int(values.at(n.second).bits()) << "b) ";
    for (auto &n : name_to_trace)
    {
      ss << n.first << "( ";
      for (auto t : traces.at(n.second))
        ss << t << " ";
      ss << ") ";
    }
    return ss.str();
  }

};

inline void to_json(json& j, const HitModel& t)
{
  j["timebase"] = t.timebase;
  for (size_t i=0; i < t.values.size(); ++i)
  {
    json jj;
    jj["name"] = t.value_names[i];
    jj["bits"] = t.values[i].bits();
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

inline void from_json(const json& j, HitModel& t)
{
  t.timebase = j["timebase"];
  if (j.count("values"))
    for (auto it : j["values"])
      t.add_value(it["name"], it["bits"].get<uint16_t>());
  if (j.count("traces"))
    for (auto it : j["traces"])
      t.add_trace(it["name"], it["dims"].get<std::vector<size_t>>());
}


}
