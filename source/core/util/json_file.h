#pragma once

#include <nlohmann/json.hpp>
#include <fstream>

inline nlohmann::json from_json_file(const std::string& file_name)
{
  std::ifstream ifs(file_name, std::ofstream::in);
  nlohmann::json j;
  if (ifs.good())
    ifs >> j;
  return j;
}

inline void to_json_file(const nlohmann::json& j, const std::string& file_name)
{
  std::ofstream(file_name, std::ofstream::trunc) << j.dump(1);
}
