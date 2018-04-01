#include "ValueDefinition.h"

#include "custom_logger.h"

ValueDefinition::ValueDefinition()
{
  std::string r{plugin_name()};

  SettingMeta valname(r + "/Name", SettingType::text, "Value name");
  valname.set_flag("preset");
  add_definition(valname);

  SettingMeta pc(r + "/PeakCenter", SettingType::floating, "Peak center (% resolution)");
  pc.set_flag("preset");
  pc.set_val("min", 0);
  pc.set_val("max", 100);
  pc.set_val("step", 0.1);
  add_definition(pc);

  SettingMeta ps(r + "/PeakSpread", SettingType::floating, "Peak spread (stddev)");
  ps.set_flag("preset");
  ps.set_val("min", 0);
  ps.set_val("step", 0.01);
  add_definition(ps);

  SettingMeta res(r + "/Resolution", SettingType::integer, "Resolution");
  res.set_flag("preset");
  res.set_val("min", 4);
  res.set_val("max", 32);
  res.set_val("units", "bits");
  add_definition(res);

  SettingMeta ptl(r + "/TraceLength", SettingType::integer, "Trace length");
  ptl.set_flag("preset");
  ptl.set_val("min", 0);
  ptl.set_val("step", 1);
  add_definition(ptl);

  int32_t i{0};
  SettingMeta val(r, SettingType::stem);
  val.set_enum(i++, r + "/Name");
  val.set_enum(i++, r + "/PeakCenter");
  val.set_enum(i++, r + "/PeakSpread");
  val.set_enum(i++, r + "/TraceLength");
  val.set_enum(i++, r + "/Resolution");

  add_definition(val);
}

Setting ValueDefinition::settings() const
{
  std::string r{plugin_name()};
  auto set = get_rich_setting(r);

  set.set(Setting::text(r + "/Name", name));
  set.set(Setting::floating(r + "/PeakCenter", center * 100));
  set.set(Setting::floating(r + "/PeakSpread", spread));
  set.set(Setting::integer(r + "/TraceLength", trace_size));
  set.set(Setting::integer(r + "/Resolution", bits_));

  return set;
}

void ValueDefinition::settings(const Setting& v)
{
  std::string r{plugin_name()};

  center = v.find({r + "/PeakCenter"}).get_number() * 0.01;
  spread = v.find({r + "/PeakSpread"}).get_number();
  trace_size = v.find({r + "/TraceLength"}).get_int();
  name = v.find({r + "/Name"}).get_text();
  bits_ = v.find({r + "/Resolution"}).get_int();
  max = pow(2, uint32_t(bits_));
}

void ValueDefinition::define(EventModel& def)
{
  dist = std::normal_distribution<double>(center * max, spread);
  def.add_value(name, max);

  if (trace_size)
    def.add_trace(name, {trace_size});
}

void ValueDefinition::generate(size_t index, Event& event)
{
  auto val = generate_val();
  event.set_value(index, val);
  if (trace_size)
    make_trace(index, event, val);
}

uint32_t ValueDefinition::generate_val()
{
  return std::round(std::max(std::min(dist(gen_), double(max)), 0.0));
}

void ValueDefinition::make_trace(size_t index, Event& e, uint32_t val)
{
  auto& trc = e.trace(index);

  size_t onset = double(trc.size()) * trace_onset;
  size_t peak = double(trc.size()) * (trace_onset + trace_risetime);

  //rise
  double slope_up = double(val) / double(peak - onset);
  for (size_t i = onset; i < peak; ++i)
    trc[i] = (i - onset) * slope_up;

  //fall
  double slope_down = double(val) / double(trc.size() * 10);
  for (size_t i = peak; i < trc.size(); ++i)
    trc[i] = val - (i - peak) * slope_down;

  // add baseline & noise
  for (size_t i = 0; i < trc.size(); ++i)
    trc[i] += trace_baseline
                  + trace_baseline ? ((rand() % trace_baseline) / 5
        - trace_baseline / 10) : 0;
}
