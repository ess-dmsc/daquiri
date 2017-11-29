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

  uint64_t start_spill(SpillQueue spill_queue) const override;
  uint64_t stop_spill(SpillQueue spill_queue) const override;
  uint64_t dummy_spill(SpillQueue spill_queue, uint64_t utime) override;
  uint64_t process_payload(SpillQueue spill_queue,
                               void*, uint64_t utime) override;


private:
  // cached params
  std::string stream_id_;
  int16_t output_channel_ {0};
  ESSGeometry geometry_;
  EventModel evt_model_;

  uint64_t latest_buf_id_ {0};

  bool eval_ordering(const EventMessage*);
  size_t event_count(const EventMessage*);
  std::string debug(const EventMessage*);
};
