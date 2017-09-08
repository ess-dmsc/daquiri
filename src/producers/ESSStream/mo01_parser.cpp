#include "mo01_parser.h"
#include "mo01_nmx_generated.h"

#include "custom_logger.h"

mo01_nmx::mo01_nmx()
  : fb_parser()
{
  SettingMeta root("mo01_nmx", SettingType::stem);
  root.set_flag("producer");
  add_definition(root);

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

void mo01_nmx::read_settings_bulk(Setting &set) const
{
  set = enrich_and_toggle_presets(set);
}

void mo01_nmx::write_settings_bulk(const Setting& settings)
{
//  auto set = enrich_and_toggle_presets(settings);
}

Spill* mo01_nmx::process_payload(void* msg, int16_t chan, TimeBase tb,
                                 uint64_t utime,
                                 PayloadStats &stats)
{
  Spill* ret {nullptr};
  auto em = GetMonitorMessage(msg);

  CustomTimer timer(true);

  ret = Spill::make_new(chan, StatusType::running);

  auto type = em->data_type();
  if (type == DataField::GEMHist)
  {
    auto hist = em->data_as_GEMHist();
    DBG << "Received GEMHist\n" << debug(*hist);
  }
  if (type == DataField::GEMTrack)
  {
    auto track = em->data_as_GEMTrack();
    DBG << "Received GEMTrack\n" << debug(*track);
  }

  stats.time_spent += timer.s();

  return ret;
}

std::string mo01_nmx::debug(const GEMHist& hist)
{
  std::stringstream ss;

  auto xhist = hist.xhist();
  if (xhist->Length())
  {
    ss << "  x: ";
    for (size_t i=0; i < xhist->Length(); ++i)
      ss << " " << xhist->Get(i);
    ss << "\n";
  }

  auto yhist = hist.yhist();
  if (yhist->Length())
  {
    ss << "  y: ";
    for (size_t i=0; i < yhist->Length(); ++i)
      ss << " " << yhist->Get(i);
    ss << "\n";
  }

  return ss.str();
}

std::string mo01_nmx::debug(const GEMTrack& track)
{
  std::stringstream ss;

  auto xtrack = track.xtrack();
  if (xtrack->Length())
  {
    ss << "  x: ";
    for (size_t i=0; i < xtrack->Length(); ++i)
    {
      auto element = xtrack->Get(i);
      ss << "(" << element->strip()
         << "," << element->time()
         << ")=" << element->adc()
         << " ";
    }
    ss << "\n";
  }

  auto ytrack = track.ytrack();
  if (ytrack->Length())
  {
    ss << "  y: ";
    for (size_t i=0; i < ytrack->Length(); ++i)
    {
      auto element = ytrack->Get(i);
      ss << "(" << element->strip()
         << "," << element->time()
         << ")=" << element->adc()
         << " ";
    }
    ss << "\n";
  }

  return ss.str();
}
