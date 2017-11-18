#include "ev42_parser.h"
#include "ev42_events_generated.h"

#include "custom_timer.h"
#include "custom_logger.h"

ev42_events::ev42_events()
: geometry_(1,1,1,1)
{
  std::string mp {"ev42_events/"};

  SettingMeta dc(mp + "OutputChannel", SettingType::integer, "Output data channel");
  dc.set_val("min", 0);
  add_definition(dc);

  SettingMeta ex(mp + "extent_x", SettingType::integer, "Extent X");
  ex.set_val("min", 1);
  add_definition(ex);

  SettingMeta ey(mp + "extent_y", SettingType::integer, "Extent Y");
  ey.set_val("min", 1);
  add_definition(ey);

  SettingMeta ez(mp + "extent_z", SettingType::integer, "Extent Z");
  ez.set_val("min", 1);
  add_definition(ez);

  SettingMeta ep(mp + "panels", SettingType::integer, "Panel count");
  ep.set_val("min", 1);
  add_definition(ep);


  SettingMeta root("ev42_events", SettingType::stem);
  root.set_flag("producer");
  root.set_enum(0, mp + "OutputChannel");
  root.set_enum(1, mp + "extent_x");
  root.set_enum(2, mp + "extent_y");
  root.set_enum(3, mp + "extent_z");
  root.set_enum(4, mp + "panels");
  add_definition(root);
}

void ev42_events::read_settings_bulk(Setting &set) const
{
  set = enrich_and_toggle_presets(set);

  set.set(Setting::integer("ev42_events/OutputChannel", output_channel_));

  set.set(Setting::integer("ev42_events/extent_x", integer_t(geometry_.nx())));
  set.set(Setting::integer("ev42_events/extent_y", integer_t(geometry_.ny())));
  set.set(Setting::integer("ev42_events/extent_z", integer_t(geometry_.nz())));
  set.set(Setting::integer("ev42_events/panels", integer_t(geometry_.np())));
}


void ev42_events::write_settings_bulk(const Setting& settings)
{
  auto set = enrich_and_toggle_presets(settings);

  output_channel_ = set.find({"ev42_events/OutputChannel"}).get_number();
  geometry_.nx(set.find({"ev42_events/extent_x"}).get_number());
  geometry_.ny(set.find({"ev42_events/extent_y"}).get_number());
  geometry_.nz(set.find({"ev42_events/extent_z"}).get_number());
  geometry_.np(set.find({"ev42_events/panels"}).get_number());

  evt_model_ = EventModel();
  evt_model_.add_value("x", geometry_.nx());
  evt_model_.add_value("y", geometry_.ny());
  evt_model_.add_value("z", geometry_.nz());
  evt_model_.add_value("panel", geometry_.np());
}

SpillPtr ev42_events::start_spill() const
{
  return Spill::make_new(StatusType::start, {output_channel_});
}

SpillPtr ev42_events::stop_spill() const
{
  return Spill::make_new(StatusType::stop, {output_channel_});
}

SpillPtr ev42_events::dummy_spill(uint64_t utime)
{
  SpillPtr ret {nullptr};

  ret = Spill::make_new(StatusType::running, {output_channel_});
  ret->stats[output_channel_].set_value("pulse_time", utime);

  stats.time_start = stats.time_end = utime;
  stats.time_spent = 0;

  return ret;
}

SpillPtr ev42_events::process_payload(void* msg, uint64_t utime)
{
  SpillPtr ret {nullptr};
  auto em = GetEventMessage(msg);

  size_t events = event_count(em);

  if (!events)
    return ret;

  CustomTimer timer(true);

  uint64_t time_high = (utime ? (utime << 32) : em->pulse_time());

  evt_model_.timebase = timebase;
  ret = Spill::make_new(StatusType::running, {output_channel_});
  ret->stats[output_channel_].set_model(evt_model_);
  ret->stats[output_channel_].set_value("pulse_time", time_high);
  //ret->stats[output_channel_].set_value("buf_id", buf_id);

  ret->events.reserve(events, Event(output_channel_, evt_model_));
  for (auto i=0; i < events; ++i)
  {
    uint64_t time = em->time_of_flight()->Get(i);
    time |= time_high;
    if (i==0)
      stats.time_start = time;
    stats.time_start = std::min(stats.time_start, time);
    stats.time_end = std::max(stats.time_end, time);

    const auto& id = em->detector_id()->Get(i);
    if (geometry_.valid_id(id)) //must be non0?
    {
      auto& evt = ret->events.last();
      evt.set_value(0, geometry_.x(id));
      evt.set_value(1, geometry_.y(id));
      evt.set_value(2, geometry_.z(id));
      evt.set_value(3, geometry_.p(id));
      evt.set_time(time);
      ++ ret->events;
    }
//    DBG << "Time " << stats.time_start << " - " << stats.time_end;
  }
  ret->events.finalize();

  stats.time_spent = timer.s();

  return ret;
}

size_t ev42_events::event_count(const EventMessage* em)
{
  //bool is_well_ordered = eval_ordering(em);
  //std::string source_name = em->source_name()->str();

  auto t_len = em->time_of_flight()->Length();
  auto p_len = em->detector_id()->Length();
  if ((t_len != p_len) || !t_len)
    return 0;

  return t_len;
}

bool ev42_events::eval_ordering(const EventMessage* em)
{
  auto buf_id = em->message_id();
  bool ret = !(buf_id < latest_buf_id_);
  if (!ret)
    WARN << "Buffer out of order (" << buf_id
         << "<" << latest_buf_id_ << ") "
         << debug(em);
  latest_buf_id_ = std::max(latest_buf_id_, buf_id);
  return ret;
}

std::string ev42_events::debug(const EventMessage* em)
{
  std::stringstream ss;

  ss << em->source_name()->str() << " #" << em->message_id()
     << " time=" << em->pulse_time()
     << " tof_size=" << em->time_of_flight()->Length()
     << " det_size=" << em->detector_id()->Length();

  return ss.str();
}
