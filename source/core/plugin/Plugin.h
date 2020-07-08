#pragma once

#include <core/plugin/Setting.h>

namespace DAQuiri {

class Plugin
{
  public:
    Plugin() = default;

    virtual ~Plugin() {}

    virtual std::string plugin_name() const = 0;

    virtual Setting settings() const = 0;
    virtual void settings(const Setting&) = 0;

  protected:
    Setting get_rich_setting(const std::string& id) const;
    void enrich(Setting& setting) const;
    void add_definition(const SettingMeta& sm);

//  private:
    std::map<std::string, SettingMeta> setting_definitions_;
};

}
