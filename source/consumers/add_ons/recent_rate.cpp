#include <consumers/add_ons/recent_rate.h>

#include <core/util/logger.h>

namespace DAQuiri {

RecentRate::RecentRate(const std::string& clock)
    : divisor_clock(clock)
{
}

void RecentRate::settings(const Setting& s)
{
  divisor_clock = s.find(Setting("divisor_clock")).get_text();
}

Setting RecentRate::settings(int32_t index) const
{
  SettingMeta divm("divisor_clock", SettingType::text, "Recent rate divisor clock");
  divm.set_flag("preset");
  divm.set_flag("native_clock");
  Setting div(divm);
  div.set_text(divisor_clock);
  div.set_indices({index});

  return div;
}

Setting RecentRate::update(const Status& current, PreciseFloat new_count)
{
  if (!previous_status.valid)
    previous_status = current;

  auto recent_t = Status::calc_diff(previous_status, current, divisor_clock);

  double recent_s = 0;
  if (recent_t != hr_duration_t())
    recent_s = std::chrono::duration_cast<std::chrono::microseconds>(recent_t).count() * 0.000001;

  if (recent_s != 0)
    current_rate = (new_count - previous_count) / recent_s;
  else
    current_rate = 0;

  previous_status = current;
  previous_count = new_count;

  SettingMeta ratem("recent_" + divisor_clock + "_rate",
                    SettingType::precise,
                    "Recent counts/" + divisor_clock + " rate");
  ratem.set_flag("readonly");
  ratem.set_val("units", "counts/s");
  Setting rate(ratem);
  rate.set_precise(current_rate);

  return rate;
}

}