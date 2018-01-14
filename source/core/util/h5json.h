#pragma once

#ifdef DAQUIRI_USE_H5
#include "json.hpp"
#include <h5cpp/hdf5.hpp>

using json = nlohmann::json;

namespace hdf5
{

//void to_json(json& j, const Enum<int16_t>& e);
//void from_json(const json& j, Enum<int16_t>& e);

void to_json(json& j, const node::Group& g);
void attr_to_json(json& j, const attribute::Attribute& g);
void to_json(json& j, const node::Dataset& d);

void from_json(const json& j, node::Group& g);

void attribute_from_json(const json& j, const std::string& name, node::Group& g);
void dataset_from_json(const json& j, const std::string& name, node::Group& g);

node::Group require_group(node::Group& g, std::string name);
bool has_group(const node::Group& g, std::string name);
bool has_dataset(const node::Group& g, std::string name);

}
#endif
