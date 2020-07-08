#pragma once

#include <core/Spill.h>

namespace DAQuiri {

struct ValueLatch
{
  public:
    void settings(const Setting& s);
    Setting settings(int32_t index = -1, std::string override_name = "") const;

    inline bool has_declared_value(const Spill& spill)
    {
      return (0 != spill.event_model.name_to_val.count(value_id));
    }

    void configure(const Spill& spill);

    template<typename T>
    inline void extract(T& bin, const Event& event) const
    {
      if (downsample)
        bin = event.value(static_cast<size_t>(idx)) >> downsample;
      else
        bin = event.value(static_cast<size_t>(idx));
    }

    inline bool valid() const
    {
      return (idx >= 0);
    }

    std::string value_id;
    uint16_t downsample {0};

  private:
    int32_t idx {-1};
};

}
