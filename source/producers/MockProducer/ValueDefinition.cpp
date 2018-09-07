#include <producers/MockProducer/ValueDefinition.h>
#include <chrono>

ValueDefinition::ValueDefinition()
{
  typedef std::chrono::system_clock myclock;
  myclock::time_point beginning = myclock::now();

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

  myclock::duration d = myclock::now() - beginning;
  gen_.seed(d.count());
}

Setting ValueDefinition::settings() const
{
  std::string r{plugin_name()};
  auto set = get_rich_setting(r);

  set.set(Setting::text(r + "/Name", name_));
  set.set(Setting::floating(r + "/PeakCenter", center_ * 100));
  set.set(Setting::floating(r + "/PeakSpread", spread_));
  set.set(Setting::integer(r + "/TraceLength", trace_length_));
  set.set(Setting::integer(r + "/Resolution", bits_));

  return set;
}

void ValueDefinition::settings(const Setting& v)
{
  std::string r{plugin_name()};

  center_ = v.find({r + "/PeakCenter"}).get_number() * 0.01;
  spread_ = v.find({r + "/PeakSpread"}).get_number();
  trace_length_ = v.find({r + "/TraceLength"}).get_int();
  name_ = v.find({r + "/Name"}).get_text();
  bits_ = v.find({r + "/Resolution"}).get_int();
  max_ = pow(2, uint32_t(bits_));
}

void ValueDefinition::define(EventModel& def)
{
  dist = std::normal_distribution<double>(center_ * max_, spread_);
  def.add_value(name_, max_);

  if (trace_length_)
    def.add_trace(name_, {trace_length_});
}

void ValueDefinition::generate(size_t index, Event& event)
{
  auto val = generate_val();
  event.set_value(index, val);
  if (trace_length_)
    make_trace(index, event, val);
}

uint32_t ValueDefinition::generate_val()
{
  return std::round(std::max(std::min(dist(gen_), double(max_)), 0.0));
}

void ValueDefinition::make_trace(size_t index, Event& e, uint32_t val)
{
  auto& trc = e.trace(index);

  size_t onset = double(trc.size()) * trace_onset_;
  size_t peak = double(trc.size()) * (trace_onset_ + trace_risetime_);

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
    trc[i] += trace_baseline_
                  + trace_baseline_ ? ((rand() % trace_baseline_) / 5
        - trace_baseline_ / 10) : 0;
}
