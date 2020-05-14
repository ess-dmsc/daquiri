#include <core/Producer.h>

namespace DAQuiri {

ProducerStatus operator|(ProducerStatus a, ProducerStatus b)
{
  return static_cast<ProducerStatus>
  (static_cast<int>(a) | static_cast<int>(b));
}

ProducerStatus operator&(ProducerStatus a, ProducerStatus b)
{
  return static_cast<ProducerStatus>
  (static_cast<int>(a) & static_cast<int>(b));
}

ProducerStatus operator^(ProducerStatus a, ProducerStatus b)
{
  return static_cast<ProducerStatus>
  (static_cast<int>(a) ^ static_cast<int>(b));
}

Setting Producer::enrich_and_toggle_presets(Setting set) const
{
  enrich(set);
  set.enable_if_flag(!(status_ & booted), "preset");
  return set;
}

}
