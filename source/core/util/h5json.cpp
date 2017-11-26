#include "h5json.h"

namespace hdf5 {

node::Group require_group(node::Group& g, std::string name)
{
  if (g.exists(name))
    node::remove(g[name]);
  return g.create_group(name);
}

bool has_group(const node::Group& g, std::string name)
{
  return (g.exists(name) &&
      (g[name].type() == node::Type::GROUP));

}

bool has_dataset(const node::Group& g, std::string name)
{
  return (g.exists(name) &&
      (g[name].type() == node::Type::DATASET));

}

//void to_json(json& j, const Enum<int16_t>& e)
//{
//  j["___choice"] = e.val();
//  std::multimap<std::string, int16_t> map;
//  for (auto a : e.options())
//    map.insert({a.second, a.first});
//  j["___options"] = json(map);
//}
//
//void from_json(const json& j, Enum<int16_t>& e)
//{
//  auto o = j["___options"];
//  for (json::iterator it = o.begin(); it != o.end(); ++it)
//    e.set_option(it.value(), it.key());
//  e.set(j["___choice"]);
//}

void to_json(json &j, const node::Group &g)
{
  for (auto n : g.nodes) {
    if (n.type() == node::Type::DATASET)
    {
      auto d = node::Dataset(n);
      json j;
      to_json(j, d);
      j[n.link().path().name()] = j;
    }
    else if (n.type() == node::Type::GROUP)
    {
      auto gg = node::Group(n);
      json j;
      to_json(j, gg);
      j[n.link().path().name()] = j;
    }
  }

  for (auto a : g.attributes)
    attr_to_json(j, a);

//  for (auto gg : g.groups())
//    to_json(j[gg], g.open_group(gg));
//  for (auto aa : g.attributes())
//    j[aa] = attribute_to_json(g, aa);
//  for (auto dd : g.datasets())
//    j[dd] = g.open_dataset(dd);
}

void attr_to_json(json &j, const attribute::Attribute &a)
{
  if (a.datatype() == datatype::create<float>()) {
    float val;
    a.read(val);
    j[a.name()] = val;
  } else if (a.datatype() == datatype::create<double>()) {
    double val;
    a.read(val);
    j[a.name()] = val;
  } else if (a.datatype() == datatype::create<long double>()) {
    long double val;
    a.read(val);
    j[a.name()] = val;
  } else if (a.datatype() == datatype::create<int8_t>()) {
    int8_t val;
    a.read(val);
    j[a.name()] = val;
  } else if (a.datatype() == datatype::create<int16_t>()) {
    int16_t val;
    a.read(val);
    j[a.name()] = val;
  } else if (a.datatype() == datatype::create<int32_t>()) {
    int32_t val;
    a.read(val);
    j[a.name()] = val;
  } else if (a.datatype() == datatype::create<int64_t>()) {
    int64_t val;
    a.read(val);
    j[a.name()] = val;
  } else if (a.datatype() == datatype::create<uint8_t>()) {
    uint8_t val;
    a.read(val);
    j[a.name()] = val;
  } else if (a.datatype() == datatype::create<uint16_t>()) {
    uint16_t val;
    a.read(val);
    j[a.name()] = val;
  } else if (a.datatype() == datatype::create<uint32_t>()) {
    uint32_t val;
    a.read(val);
    j[a.name()] = val;
  } else if (a.datatype() == datatype::create<uint64_t>()) {
    uint64_t val;
    a.read(val);
    j[a.name()] = val;
  } else if (a.datatype() == datatype::create<std::string>()) {
    std::string val;
    a.read(val);
    j[a.name()] = val;
  }
//  else if (a.datatype() == datatype::create<bool>()) {
//    bool val;
//    a.read(val);
//    j[a.name()] = val;
//  }
//  else if (g.template attr_is_enum<int16_t>())
//    return json(g.template read_enum<int16_t>());
//  else
//    return "ERROR: to_json unimplemented attribute type";
}

void attribute_from_json(const json &j, const std::string &name,
                         node::Group &g)
{
//  if (j.count("___options") && j.count("___choice"))
//  {
//    Enum<int16_t> e = j;
//    g.write_enum(name, e);
//  }
//  else
  if (j.is_number_float()) {
    attribute::Attribute a = g.attributes.create<double>(name);
    a.write(j.get<double>());
  } else if (j.is_number_unsigned()) {
    attribute::Attribute a = g.attributes.create<uint32_t>(name);
    a.write(j.get<uint32_t>());
  } else if (j.is_number_integer()) {
    attribute::Attribute a = g.attributes.create<int64_t>(name);
    a.write(j.get<int64_t>());
//  } else if (j.is_boolean()) {
//    attribute::Attribute a = g.attributes.create<bool>(name);
//    a.write(j.get<bool>());
  } else if (j.is_string()) {
    attribute::Attribute a = g.attributes.create<std::string>(name);
    a.write(j.get<std::string>());
  }
}

void from_json(const json &j, node::Group &g)
{
  bool is_array = j.is_array();
  uint32_t i = 0;
  size_t len = 0;
  if (is_array && j.size())
    len = std::to_string(j.size() - 1).size();

  for (json::const_iterator it = j.begin(); it != j.end(); ++it) {
    std::string name;
    if (is_array) {
      name = std::to_string(i++);
      if (name.size() < len)
        name = std::string(len - name.size(), '0').append(name);
    } else
      name = it.key();

    if (it.value().count("___shape") && it.value()["___shape"].is_array()) {
      dataset_from_json(it.value(), name, g);
    } else if (!it.value().is_array() &&
        (it.value().is_number() ||
            it.value().is_boolean() ||
            it.value().is_string() ||
            it.value().count("___options") ||
            it.value().count("___choice"))) {
      attribute_from_json(it.value(), name, g);
    } else {
      auto gg = g.create_group(name);
      from_json(it.value(), gg);
    }
  }
}

void to_json(json &j, const node::Dataset &d)
{
  auto dsp = dataspace::Simple(d.dataspace());
  auto dims = dsp.current_dimensions();
  auto maxdims = dsp.maximum_dimensions();
  j["___shape"] = dims;
  if (dims != maxdims)
    j["___extends"] = maxdims;
  auto cl = d.creation_list();
  if (cl.layout() == property::DatasetLayout::CHUNKED)
    j["___chunk"] = cl.chunk();

  for (auto a : d.attributes)
    attr_to_json(j, a);
}

void dataset_from_json(const json &j,
                       __attribute__((unused)) const std::string &name,
                       __attribute__((unused)) node::Group &g)
{
  std::vector<hsize_t> shape = j["___shape"];

  std::vector<hsize_t> extends;
  if (j.count("___extends") && j["___extends"].is_array())
    extends = j["___extends"].get<std::vector<hsize_t>>();

  std::vector<hsize_t> chunk;
  if (j.count("___chunk") && j["___chunk"].is_array())
    chunk = j["___chunk"].get<std::vector<hsize_t>>();
//  auto dset = g.create_dataset<int>(name, shape, chunk);
//  for (json::const_iterator it = j.begin(); it != j.end(); ++it)
//  {
//    attribute_from_json(json(it.value()), std::string(it.key()), dset);
//  }
}

}
