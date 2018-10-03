#include "UnitConverter.h"

#include <sstream>
#include <iomanip>
#include <numeric>
#include <core/util/string_extensions.h>
#include <core/util/custom_logger.h>

UnitConverter::UnitConverter()
{
//  add_prefix("P", 1E12f);
//  add_prefix("T", 1E9f);
  add_prefix("M", 1E6f);
  add_prefix("k", 1E3f);
  prefix_values_indexed.push_back("");
//  add_prefix("c", 1E-2f);
  add_prefix("m", 1E-3f);
  add_prefix("\u03BC", 1E-6f);
  prefix_values["u"] = 1E-6f;
  add_prefix("n", 1E-9f);
  add_prefix("p", 1E-12f);
//  add_prefix("f", 1E-15f);
}

void UnitConverter::add_prefix(std::string prefix, PreciseFloat factor)
{
  if (!prefix_values.count(prefix))
  {
    prefix_values[prefix] = factor;
    prefix_values_indexed.push_back(prefix);
  }
}

std::map<int32_t, std::string> UnitConverter::make_ordered_map(std::string default_unit,
                                                               PreciseFloat min_SI_value,
                                                               PreciseFloat max_SI_value) const
{
  std::string stripped_unit = strip_unit(default_unit);
  std::string default_prefix = get_prefix(default_unit);

  if (min_SI_value < 0)
    min_SI_value = -min_SI_value;

  if (max_SI_value < 0)
    max_SI_value = -max_SI_value;

  std::map<int32_t, std::string> newmap;
  for (std::size_t i=0; i < prefix_values_indexed.size(); ++i)
  {
    bool min_criterion = (convert_prefix(min_SI_value, default_prefix, prefix_values_indexed[i]) >= 0.00001);
    bool max_criterion = (convert_prefix(max_SI_value, default_prefix, prefix_values_indexed[i]) <= 999999);
    if ((min_SI_value && min_criterion && max_SI_value && max_criterion)
        || (!min_SI_value && max_SI_value && max_criterion)
        || (!max_SI_value && min_SI_value && min_criterion))
    {
      std::string full_unit = prefix_values_indexed[i] + stripped_unit;
      newmap[i] = full_unit;
    }
  }
  return newmap;
}

PreciseFloat UnitConverter::convert_units(PreciseFloat value, std::string from, std::string to) const
{
  trim(from);
  trim(to);
  if (from == to)
    return value;
  if (strip_unit(from) != strip_unit(to))
    return value;
  return convert_prefix(value, get_prefix(from), get_prefix(to));
}

std::string UnitConverter::get_prefix(std::string full_unit) const
{
//  std::u32string ucs4 = boost::locale::conv::utf_to_utf<char32_t>(full_unit);
//  std::u32string prefix = ucs4;

  auto prefix = full_unit;
  if (full_unit.size() > 1)
    prefix = full_unit.substr(0, 1);
//  return boost::locale::conv::utf_to_utf<char>(prefix);
  return prefix;
}

std::string UnitConverter::strip_unit(std::string full_unit) const
{
//  std::u32string ucs4 = boost::locale::conv::utf_to_utf<char32_t>(full_unit);
//  std::u32string stripped_unit = ucs4;

  auto stripped_unit = full_unit;
  if (full_unit.size() > 1)
    stripped_unit = full_unit.substr(1, full_unit.size()-1);
//  return boost::locale::conv::utf_to_utf<char>(stripped_unit);
  return stripped_unit;
}

PreciseFloat UnitConverter::convert_prefix(PreciseFloat value, std::string from, std::string to) const
{
  if (!from.empty() && prefix_values.count(from))
    value *= prefix_values.at(from);
  if (!to.empty() && prefix_values.count(to))
    value /= prefix_values.at(to);
  return value;
}
