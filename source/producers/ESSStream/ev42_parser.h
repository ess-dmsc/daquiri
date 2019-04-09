#pragma once

#include <producers/ESSStream/fb_parser.h>
#include <producers/ESSStream/ESSGeometryPlugin.h>

using namespace DAQuiri;

class EventMessage;

class ev42_events : public fb_parser
{
 public:
  ev42_events();

  ~ev42_events() = default;

  std::string plugin_name() const override
  { return "ev42_events"; }

  std::string schema_id() const override;
  std::string get_source_name(void* msg) const override;

  void settings(const Setting&) override;
  Setting settings() const override;

  uint64_t process_payload(SpillQueue spill_queue, void* msg) override;
  uint64_t stop(SpillQueue spill_queue) override;

  StreamManifest stream_manifest() const override;

 private:
  // cached params

  enum Spoof : int32_t
  {
    None = 0,
    Monotonous = 1,
    Earliest = 2
  };

  enum CheckOrdering : int32_t
  {
    Ignore = 0,
    Warn = 1,
    Reject = 2
  };

  std::string stream_id_;
  ESSGeometryPlugin geometry_;
  EventModel event_definition_;
  Spoof spoof_clock_{None};
  bool heartbeat_{false};

  bool filter_source_name_{false};
  std::string source_name_;
  CheckOrdering ordering_{Ignore};

  // to ensure expected stream structure
  bool started_{false};
  uint64_t spoofed_time_{0};

  // stream error checking
  uint64_t latest_buf_id_{0};

  bool in_order(const EventMessage*);
  size_t events_in_buffer(const EventMessage*);
  std::string debug(const EventMessage*);
};
