#pragma once

#include <vector>
#include <map>
#include <string>
#include "precise_float.h"

class UnitConverter
{
public:
  UnitConverter();

  std::string strip_unit(std::string full_unit) const;
  std::string get_prefix(std::string full_unit) const;

  PreciseFloat convert_units(PreciseFloat value, std::string from, std::string to) const;
  PreciseFloat convert_prefix(PreciseFloat value, std::string from, std::string to) const;

  std::map<int32_t, std::string> make_ordered_map(std::string stripped_unit, PreciseFloat min_SI_value, PreciseFloat max_SI_value) const;

  void add_prefix(std::string prefix, PreciseFloat factor);

  std::map<std::string, PreciseFloat> prefix_values;
  std::vector<std::string> prefix_values_indexed;
};
