#include "MockProducer.h"
#include "custom_timer.h"

#include "custom_logger.h"

void ValueDefinition::define(EventModel& def)
{
  dist = std::normal_distribution<double>(center * max, spread);
  def.add_value(name, max);

  if (trace_size)
    def.add_trace(name, {trace_size});
}

void ValueDefinition::generate(size_t index, Event& event, std::default_random_engine& gen)
{
  auto val = generate(gen);
  event.set_value(index, val);
  if (trace_size)
    make_trace(index, event, val);
}

uint32_t ValueDefinition::generate(std::default_random_engine& gen)
{
  return std::round(std::max(std::min(dist(gen), double(max)), 0.0));
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

MockProducer::MockProducer()
{
  std::string r{plugin_name()};

  SettingMeta streamid(r + "/StreamID", SettingType::text, "DAQuiri stream ID");
  streamid.set_flag("preset");
  add_definition(streamid);

  SettingMeta si(r + "/SpillInterval", SettingType::floating, "Interval between spills");
  si.set_val("min", 0.001);
  si.set_val("max", 1000);
  si.set_val("step", 0.001);
  si.set_val("units", "s");
  add_definition(si);

  SettingMeta res(r + "/Resolution", SettingType::integer, "Resolution");
  res.set_val("min", 4);
  res.set_val("max", 32);
  res.set_val("units", "bits");
  add_definition(res);

  SettingMeta cr(r + "/CountRate", SettingType::floating, "Event rate");
  cr.set_val("min", 1);
  si.set_val("step", 1);
  cr.set_val("units", "cps");
  add_definition(cr);

  SettingMeta tm(r + "/TimebaseMult", SettingType::integer, "Timebase multiplier");
  tm.set_val("min", 1);
  tm.set_val("units", "ns");
  add_definition(tm);

  SettingMeta td(r + "/TimebaseDiv", SettingType::integer, "Timebase divider");
  td.set_val("min", 1);
  td.set_val("units", "1/ns");
  add_definition(td);

  SettingMeta dt(r + "/DeadTime", SettingType::floating, "Dead time (% of real time)");
  dt.set_val("min", 0);
  dt.set_val("max", 100);
  dt.set_val("step", 0.01);
  add_definition(dt);

  SettingMeta lambda(r + "/Lambda", SettingType::floating, "Decay constant (λ)");
  lambda.set_val("min", 0);
  lambda.set_val("step", 0.01);
  add_definition(lambda);

  SettingMeta lambda2(r + "/SpillLambda", SettingType::floating, "Decay constant (λ) per spill");
  lambda2.set_val("min", 0);
  lambda2.set_val("max", 100);
  lambda2.set_val("units", "%");
  lambda2.set_val("step", 0.1);
  add_definition(lambda2);

  SettingMeta vc(r + "/ValueCount", SettingType::integer, "Value count");
  vc.set_flag("preset");
  vc.set_val("min", 1);
  vc.set_val("max", 16);
  add_definition(vc);

  SettingMeta valname(r + "/Value/Name", SettingType::text, "Value name");
  valname.set_flag("preset");
  add_definition(valname);

  SettingMeta pc(r + "/Value/PeakCenter", SettingType::floating, "Peak center (% resolution)");
  pc.set_flag("preset");
  pc.set_val("min", 0);
  pc.set_val("max", 100);
  pc.set_val("step", 0.1);
  add_definition(pc);

  SettingMeta ps(r + "/Value/PeakSpread", SettingType::floating, "Peak spread (stddev)");
  ps.set_flag("preset");
  ps.set_val("min", 0);
  ps.set_val("step", 0.01);
  add_definition(ps);

  SettingMeta ptl(r + "/Value/TraceLength", SettingType::integer, "Trace length");
  ptl.set_flag("preset");
  ptl.set_val("min", 0);
  ptl.set_val("step", 1);
  add_definition(ptl);

  SettingMeta val(r + "/Value", SettingType::stem);
  val.set_enum(0, r + "/Value/Name");
  val.set_enum(1, r + "/Value/PeakCenter");
  val.set_enum(2, r + "/Value/PeakSpread");
  val.set_enum(3, r + "/Value/TraceLength");
  add_definition(val);

  SettingMeta root("MockProducer", SettingType::stem);
  root.set_flag("producer");
  root.set_enum(0, r + "/StreamID");
  root.set_enum(1, r + "/SpillInterval");
  root.set_enum(2, r + "/Resolution");
  root.set_enum(3, r + "/CountRate");
  root.set_enum(4, r + "/DeadTime");
  root.set_enum(5, r + "/TimebaseMult");
  root.set_enum(6, r + "/TimebaseDiv");
  root.set_enum(7, r + "/Lambda");
  root.set_enum(8, r + "/SpillLambda");
  root.set_enum(9, r + "/ValueCount");
  add_definition(root);

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

MockProducer::~MockProducer()
{
  daq_stop();
  die();
}

StreamManifest MockProducer::stream_manifest() const
{
  StreamManifest ret;
  ret[stream_id_] = event_definition_;
  return ret;
}

bool MockProducer::daq_start(SpillQueue out_queue)
{
  if (running_.load())
    daq_stop();

  terminate_.store(false);
  running_.store(true);
  runner_ = std::thread(&MockProducer::worker_run, this, out_queue);

  return true;
}

bool MockProducer::daq_stop()
{
  terminate_.store(true);

  if (runner_.joinable())
    runner_.join();
  running_.store(false);

  return true;
}

bool MockProducer::daq_running()
{
  return (running_.load());
}

Setting MockProducer::settings() const
{
  std::string r{plugin_name()};
  auto set = get_rich_setting(r);

  set.set(Setting::text(r + "/StreamID", stream_id_));
  set.set(Setting::floating(r + "/SpillInterval", spill_interval_));
  set.set(Setting::integer(r + "/Resolution", bits_));
  set.set(Setting::floating(r + "/CountRate", count_rate_));
  set.set(Setting::floating(r + "/DeadTime", dead_ * 100.0));
  set.set(Setting::integer(r + "/TimebaseMult", event_definition_.timebase.multiplier()));
  set.set(Setting::integer(r + "/TimebaseDiv", event_definition_.timebase.divider()));
  set.set(Setting::floating(r + "/Lambda", lambda_));
  set.set(Setting::floating(r + "/SpillLambda", spill_lambda_));
  set.set(Setting::integer(r + "/ValueCount", integer_t(val_defs_.size())));

  while (set.branches.has_a(Setting({r + "/Value", SettingType::stem})))
    set.branches.remove_a(Setting({r + "/Value", SettingType::stem}));

  for (int i = 0; i < int(val_defs_.size()); ++i)
  {
    Setting v = get_rich_setting(r + "/Value");
    v.set_indices({i});
    v.branches = get_rich_setting(r + "/Value").branches;
    v.set(Setting::text(r + "/Value/Name", val_defs_[i].name));
    v.set(Setting::floating(r + "/Value/PeakCenter", val_defs_[i].center * 100));
    v.set(Setting::floating(r + "/Value/PeakSpread", val_defs_[i].spread));
    v.set(Setting::integer(r + "/Value/TraceLength", val_defs_[i].trace_size));
    for (auto& vv : v.branches)
      vv.set_indices({i});
    set.branches.add_a(v);
  }

  set.enable_if_flag(!(status_ & booted), "preset");
  return set;
}

void MockProducer::settings(const Setting& settings)
{
  std::string r{plugin_name()};
  auto set = enrich_and_toggle_presets(settings);

  stream_id_ = set.find({r + "/StreamID"}).get_text();
  spill_interval_ = set.find({r + "/SpillInterval"}).get_number();
  bits_ = set.find({r + "/Resolution"}).get_number();
  count_rate_ = set.find({r + "/CountRate"}).get_number();
  lambda_ = set.find({r + "/Lambda"}).get_number();
  spill_lambda_ = set.find({r + "/SpillLambda"}).get_number();
  dead_ = set.find({r + "/DeadTime"}).get_number() * 0.01;

  uint16_t val_count_ = std::max(int(set.find({r + "/ValueCount"}).get_number()), 1);

  if (val_defs_.size() != val_count_)
    val_defs_.resize(val_count_);

  uint32_t resolution = pow(2, uint32_t(bits_));

  for (Setting v : set.branches)
  {
    if (v.id() != r + "/Value")
      continue;
    auto indices = v.indices();
    if (!indices.size())
      continue;
    size_t idx = *indices.begin();
//    DBG << "Write idx " << idx;
    if (idx >= val_defs_.size())
      continue;
    val_defs_[idx].center = v.find({r + "/Value/PeakCenter"}).get_number() * 0.01;
    val_defs_[idx].spread = v.find({r + "/Value/PeakSpread"}).get_number();
    val_defs_[idx].trace_size = v.find({r + "/Value/TraceLength"}).get_number();
    val_defs_[idx].name = v.find({r + "/Value/Name"}).get_text();
    val_defs_[idx].max = resolution;
  }

  event_definition_ = EventModel();
  event_definition_.timebase = TimeBase(set.find({r + "/TimebaseMult"}).get_number(),
                                        set.find({r + "/TimebaseDiv"}).get_number());

  for (size_t i = 0; i < val_defs_.size(); ++i)
    val_defs_[i].define(event_definition_);
}

void MockProducer::boot()
{
  if (!(status_ & ProducerStatus::can_boot))
  {
    WARN << "<MockProducer> Cannot boot MockProducer. Failed flag check (can_boot == 0)";
    return;
  }

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;

  INFO << "<MockProducer> Booting"
       << " rate=" << count_rate_ << "cps";
//        << " with peak at " << peak_center_ << "%   stdev=" << peak_spread_;


  clock_ = 0;
  status_ = ProducerStatus::loaded | ProducerStatus::booted | ProducerStatus::can_run;
}

void MockProducer::die()
{
//  INFO << "<MockProducer> Shutting down";
  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

void MockProducer::worker_run(SpillQueue spill_queue)
{
  DBG << "<MockProducer> Starting run   "
      << "  timebase " << event_definition_.timebase.debug() << "ns"
      << "  init_rate=" << count_rate_ << "cps"
      << "  lambda=" << lambda_;

  CustomTimer timer(true);

  spill_queue->enqueue(get_spill(StatusType::start, timer.s()));
  while (!terminate_.load())
  {
    spill_queue->enqueue(get_spill(StatusType::running, timer.s()));
    wait_ms(spill_interval_ * 1000.0);

    double seconds = timer.s();
    double overshoot = ((event_definition_.timebase.to_sec(clock_) - seconds) / seconds * 100.0);
    if (overshoot > 0)
      DBG << "<MockProducer> Native clock overshoot " << overshoot << "%";
  }
  spill_queue->enqueue(get_spill(StatusType::stop, timer.s()));
}

void MockProducer::add_hit(Spill& spill, uint64_t time)
{
  auto& e = spill.events.last();
  e.set_time(time);
  for (size_t i = 0; i < val_defs_.size(); ++i)
    val_defs_[i].generate(i, e, gen_);
  ++spill.events;
}

SpillPtr MockProducer::get_spill(StatusType t, double seconds)
{
  SpillPtr spill = std::make_shared<Spill>(stream_id_, t);

  recent_pulse_time_ = clock_ = event_definition_.timebase.to_native(seconds * pow(10, 9));

  if (t == StatusType::running)
    fill_events(spill, seconds);
  spill->events.finalize();

  clock_ = event_definition_.timebase.to_native((seconds + spill_interval_) * pow(10, 9));
  fill_stats(*spill);

  return spill;
}

void MockProducer::fill_events(SpillPtr& spill, double seconds)
{
  double rate = count_rate_;
  if (lambda_)
    rate *= exp(0.0 - lambda_ * seconds);

  uint32_t total_events = (rate * spill_interval_) * (1.0 - dead_);
  double event_interval = event_definition_.timebase.to_native(spill_interval_ * pow(10, 9)) / total_events;

  spill->events.reserve(total_events, event_definition_);
  double time_bonus{0};
  for (uint32_t i = 0; i < total_events; i++)
  {
    if ((spill_lambda_ >= 100) || eval_spill_lambda(i, total_events))
      add_hit(*spill, clock_ + time_bonus);
    time_bonus += event_interval;
  }
}

bool MockProducer::eval_spill_lambda(uint32_t i, uint32_t total)
{
  double diff = (spill_lambda_ * 0.01) +
      (1.0 - (spill_lambda_ * 0.01)) *
          (double(total - i) / double(total));
  return (event_chance_(gen_) < diff);
}

void MockProducer::fill_stats(Spill& spill) const
{
  spill.event_model = event_definition_;

  double duration = clock_;
  double duration_live = duration * (1.0 - dead_);
  double duration_trigger = duration * (1.0 - 0.5 * dead_);

  spill.state.branches.add(Setting::floating("native_time", duration));
  spill.state.branches.add(Setting::floating("live_time", duration_live));
  spill.state.branches.add(Setting::floating("live_trigger", duration_trigger));
  spill.state.branches.add(Setting::floating("pulse_time", double(recent_pulse_time_)));
}
