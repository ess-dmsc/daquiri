#pragma once

#include "fb_parser.h"
#include "ESSGeometry.h"

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

  uint64_t process_payload(SpillQueue spill_queue, void* msg) override;
  uint64_t stop(SpillQueue spill_queue) override;

  StreamManifest stream_manifest() const override;

private:
  // cached params

  std::string stream_id_;
  ESSGeometry geometry_ {1,1,1,1};
  EventModel event_definition_;
  int spoof_clock_ {0};
  bool heartbeat_ {false};

  bool filter_source_name_ {false};
  std::string source_name_;
  int ordering_ {0};

  // to ensure expected stream structure
  bool started_ {false};
  uint64_t spoofed_time_ {0};

  // stream error checking
  uint64_t latest_buf_id_ {0};

  bool in_order(const EventMessage*);
  size_t events_in_buffer(const EventMessage*);
  std::string debug(const EventMessage*);
};
