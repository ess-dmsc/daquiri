/*******************************************************************************
 *
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 *
 * This software can be redistributed and/or modified freely provided that
 * any derivative works bear some notice that they are derived from it, and
 * any modified versions bear some notice that they have been modified.
 *
 * Author(s):
 *      Martin Shetty (NIST)
 *
 ******************************************************************************/

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
    for (auto &h : e.hits)
    {
      if ((h.first < 0) || (h.first >= static_cast<int16_t>(gates_.size())))
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
    for (auto &h : e.hits)
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
