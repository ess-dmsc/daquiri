#pragma once

#include <h5cpp/hdf5.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace hdf5 {

// void to_json(json& j, const Enum<int16_t>& e);
// void from_json(const json& j, Enum<int16_t>& e);

void to_json(json &j, const node::Group &g);
void attr_to_json(json &j, const attribute::Attribute &g);
void to_json(json &j, const node::Dataset &d);

void from_json(const json &j, node::Group &g);

void attribute_from_json(const json &j, const std::string &name,
                         node::Group &g);
void dataset_from_json(const json &j, const std::string &name, node::Group &g);

node::Group require_group(node::Group &g, std::string name);

} // namespace hdf5

std::string vector_idx_minlen(size_t idx, size_t max);
