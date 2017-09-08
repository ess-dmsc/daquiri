#pragma once

#include "fb_parser.h"
#include "ess_geometry.h"
#include "custom_timer.h"

using namespace DAQuiri;

class EventMessage;

class ev42_events : public fb_parser
{
public:
  ev42_events();
  ~ev42_events() {}

  std::string plugin_name() const override {return "ev42_events";}

  void write_settings_bulk(const Setting&) override;
  void read_settings_bulk(Setting&) const override;

  Spill* process_payload(void*, int16_t chan, TimeBase tb,
                         PayloadStats& stats) override;

private:
  // cached params
  GeometryInterpreter geometry_;

  uint64_t latest_buf_id_ {0};

  std::string debug(const EventMessage&);
};
