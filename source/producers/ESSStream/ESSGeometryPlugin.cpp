#include "ESSGeometryPlugin.h"

ESSGeometryPlugin::ESSGeometryPlugin()
{
  std::string r{plugin_name()};

  SettingMeta ex(r + "/extent_x", SettingType::integer, "Extent X");
  ex.set_val("min", 1);
  add_definition(ex);

  SettingMeta ey(r + "/extent_y", SettingType::integer, "Extent Y");
  ey.set_val("min", 1);
  add_definition(ey);

  SettingMeta ez(r + "/extent_z", SettingType::integer, "Extent Z");
  ez.set_val("min", 1);
  add_definition(ez);

  SettingMeta ep(r + "/panels", SettingType::integer, "Panel count");
  ep.set_val("min", 1);
  add_definition(ep);

  int32_t i{0};
  SettingMeta root(r, SettingType::stem, "Logical geometry");
  root.set_enum(i++, r + "/extent_x");
  root.set_enum(i++, r + "/extent_y");
  root.set_enum(i++, r + "/extent_z");
  root.set_enum(i++, r + "/panels");
  add_definition(root);
}

Setting ESSGeometryPlugin::settings() const
{
  std::string r{plugin_name()};
  auto set = get_rich_setting(r);
  set.set(Setting::integer(r + "/extent_x", integer_t(geometry_.nx())));
  set.set(Setting::integer(r + "/extent_y", integer_t(geometry_.ny())));
  set.set(Setting::integer(r + "/extent_z", integer_t(geometry_.nz())));
  set.set(Setting::integer(r + "/panels", integer_t(geometry_.np())));
  return set;
}

void ESSGeometryPlugin::settings(const Setting& settings)
{
  std::string r{plugin_name()};
  geometry_.nx(settings.find({r + "/extent_x"}).get_number());
  geometry_.ny(settings.find({r + "/extent_y"}).get_number());
  geometry_.nz(settings.find({r + "/extent_z"}).get_number());
  geometry_.np(settings.find({r + "/panels"}).get_number());
}

void ESSGeometryPlugin::define(EventModel& definition)
{
  definition.add_value("x", geometry_.nx());
  definition.add_value("y", geometry_.ny());
  definition.add_value("z", geometry_.nz());
  definition.add_value("panel", geometry_.np());
}

bool ESSGeometryPlugin::fill(Event& event, uint32_t pixel_id)
{
  if (!geometry_.valid_id(pixel_id)) //must be non0?
    return false;
  event.set_value(0, geometry_.x(pixel_id));
  event.set_value(1, geometry_.y(pixel_id));
  event.set_value(2, geometry_.z(pixel_id));
  event.set_value(3, geometry_.p(pixel_id));
  return true;
}