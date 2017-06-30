#pragma once

#include <vector>
#include <string>
#include "event.h"

namespace DAQuiri {

class Pattern
{
private:
  std::vector<bool> gates_;
  size_t threshold_ {0};

public:
  inline Pattern() {}

  inline Pattern(const std::string &s)
  {
    from_string(s);
  }

  void resize(size_t);
  std::vector<bool> gates() const { return gates_; }
  size_t threshold() const { return threshold_; }
  void set_gates(std::vector<bool>);
  void set_theshold(size_t);

  inline bool relevant(size_t chan) const
  {
    if (chan >= gates_.size())
      return false;
    return gates_[chan];
  }

  inline bool validate(const Event &e) const
  {
    if (threshold_ == 0)
      return true;
    size_t matches = 0;
    for (auto h : e.hits())
    {
      if ((h.first < 0) ||
          (h.first >= static_cast<int16_t>(gates_.size())))
        continue;
      else if (gates_[h.first])
        matches++;
      if (matches == threshold_)
        break;
    }
    return (matches == threshold_);
  }

  inline bool antivalidate(const Event &e) const
  {
    if (threshold_ == 0)
      return true;
    size_t matches = threshold_;
    for (auto h : e.hits())
    {
      if ((h.first < 0) || (h.first >= static_cast<int16_t>(gates_.size())))
        continue;
      else if (gates_[h.first])
        matches--;
      if (matches < threshold_)
        break;
    }
    return (matches == threshold_);
  }

  std::string to_string() const;
  void from_string(std::string s);

  std::string gates_to_string() const;
  void gates_from_string(std::string s);

  bool operator==(const Pattern other) const;
  bool operator!=(const Pattern other) const {return !operator ==(other);}
};

void to_json(json& j, const Pattern &s);
void from_json(const json& j, Pattern &s);

}
