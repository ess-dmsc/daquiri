#pragma once

#include <core/plugin/Plugin.h>
#include <core/Event.h>
#include <logical_geometry/ESSGeometry.h>

using namespace DAQuiri;

class ESSGeometryPlugin : public Plugin
{
  public:
    ESSGeometryPlugin();

    std::string plugin_name() const override { return "ESSGeometry"; }
    Setting settings() const override;
    void settings(const Setting&) override;

    void define(EventModel& definition);
    bool fill(Event& event, uint32_t pixel_id);

  private:
    ESSGeometry geometry_{1, 1, 1, 1};
};
