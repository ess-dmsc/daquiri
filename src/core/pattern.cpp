#include "pattern.h"
#include <boost/algorithm/string.hpp>
#include <sstream>

namespace DAQuiri {

void Pattern::resize(size_t sz)
{
  gates_.resize(sz);
  if (threshold_ > sz)
    threshold_ = sz;
}

void Pattern::set_gates(std::vector<bool> gts)
{
  gates_ = gts;
  if (threshold_ > gates_.size())
    threshold_ = gates_.size();
}

void Pattern::set_theshold(size_t sz)
{
  threshold_ = sz;
  if (threshold_ > gates_.size())
    threshold_ = gates_.size();
}

bool Pattern::operator==(const Pattern other) const
{
  if (gates_ != other.gates_)
    return false;
  if (threshold_ != other.threshold_)
    return false;
  return true;
}

std::string Pattern::gates_to_string() const
{
  std::string gts;
  for (bool g : gates_)
    if (g)
      gts += "+";
    else
      gts += "o";
  return gts;
}

void Pattern::gates_from_string(std::string s)
{
  std::vector<bool> gts;
  if (s.size())
  {
    boost::algorithm::trim(s);
    if (s.size())
    {
      for (char &c : s)
        if (c == '+')
          gts.push_back(true);
        else
          gts.push_back(false);
    }
  }
  gates_ = gts;
}

std::string Pattern::to_string() const
{
  std::stringstream ss;
  ss << threshold_;
  return ss.str() + gates_to_string();
}

void Pattern::from_string(std::string s)
{
  threshold_ = 0;
  std::stringstream ss(s);
  ss >> threshold_;
  std::string gts;
  ss >> gts;
  gates_from_string(gts);
}

void to_json(json& j, const Pattern &s)
{
  j["threshold"] = s.threshold();
  j["gates"] = s.gates_to_string();
}

void from_json(const json& j, Pattern &s)
{
  s.set_theshold(j["threshold"]);
  s.gates_from_string(j["gates"]);
}


}
