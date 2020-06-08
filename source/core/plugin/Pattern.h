#pragma once

#include <vector>
#include <string>
#include <sstream>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace DAQuiri {

class Pattern
{
public:
  inline Pattern() {}
  inline Pattern(size_t thresh, std::vector<bool> gts)
  {
    set(thresh, gts);
  }

  inline const std::vector<bool>& gates() const { return gates_; }
  inline size_t threshold() const { return threshold_; }

  inline bool relevant(size_t chan) const
  {
    return ((chan < gates_.size()) && gates_[chan]);
  }

  inline void resize(size_t sz)
  {
    gates_.resize(sz);
    threshold_ = std::min(sz, threshold_);
  }

  inline void set_gates(std::vector<bool> gts)
  {
    gates_ = gts;
    threshold_ = std::min(gates_.size(), threshold_);
  }

  inline void set_threshold(size_t sz)
  {
    threshold_ = sz;
    threshold_ = std::min(gates_.size(), threshold_);
  }

  inline void set(size_t thresh, std::vector<bool> gts)
  {
    threshold_ = thresh;
    gates_ = gts;
    threshold_ = std::min(gates_.size(), threshold_);
  }

  inline bool operator==(const Pattern other) const
  {
    return ((gates_ == other.gates_) &&
            (threshold_ == other.threshold_));
  }

  inline bool operator!=(const Pattern other) const
  {
    return !operator ==(other);
  }

  inline std::string debug() const
  {
    std::stringstream ss;
    ss << threshold_;
    for (bool g : gates_)
      if (g)
        ss << "+";
      else
        ss << "o";
    return ss.str();
  }

private:
  std::vector<bool> gates_;
  size_t threshold_ {0};
};

inline void to_json(json& j, const Pattern &s)
{
  j["threshold"] = s.threshold();
  json g;
  for (auto a : s.gates())
    g.push_back(int(a));  //hack for OSX
  j["gates"] = g;
}

inline void from_json(const json& j, Pattern &s)
{
  std::vector<bool> gates;
  if (j.count("gates"))
    for (auto g : j["gates"])
      gates.push_back(g != 0);
  s.set_gates(gates);
  if (j.count("threshold"))
    s.set_threshold(j["threshold"]);
}

}
