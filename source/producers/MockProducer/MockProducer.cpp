#include <producers/MockProducer/MockProducer.h>
#include <core/util/timer.h>

#include <core/util/logger.h>

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

  SettingMeta cr(r + "/CountRate", SettingType::floating, "Event rate");
  cr.set_val("min", 1);
  cr.set_val("step", 1);
  cr.set_val("units", "cps");
  add_definition(cr);

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

  int32_t i{0};
  SettingMeta root(r, SettingType::stem);
  root.set_flag("producer");
  root.set_enum(i++, r + "/StreamID");
  root.set_enum(i++, r + "/SpillInterval");
  root.set_enum(i++, r + "/CountRate");
  root.set_enum(i++, r + "/DeadTime");
  root.set_enum(i++, r + "/Lambda");
  root.set_enum(i++, r + "/SpillLambda");
  root.set_enum(i++, r + "/ValueCount");
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
  ret[stream_id_].event_model = event_definition_;
  ret[stream_id_].stats.branches.add(SettingMeta("native_time", SettingType::floating));
  ret[stream_id_].stats.branches.add(SettingMeta("live_time", SettingType::floating));
  ret[stream_id_].stats.branches.add(SettingMeta("live_trigger", SettingType::floating));
  ret[stream_id_].stats.branches.add(SettingMeta("pulse_time", SettingType::floating));

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
  set.set(Setting::floating(r + "/CountRate", count_rate_));
  set.set(Setting::floating(r + "/DeadTime", dead_ * 100.0));
  set.set(Setting::floating(r + "/Lambda", lambda_));
  set.set(Setting::floating(r + "/SpillLambda", spill_lambda_));

  set.branches.add_a(TimeBasePlugin(event_definition_.timebase).settings());

  set.set(Setting::integer(r + "/ValueCount", integer_t(val_defs_.size())));
  while (set.branches.has_a(Setting({r + "/Value", SettingType::stem})))
    set.branches.remove_a(Setting({r + "/Value", SettingType::stem}));
  for (int i = 0; i < int(val_defs_.size()); ++i)
  {
    Setting v = val_defs_[i].settings();
    v.set_indices({i});
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
  count_rate_ = set.find({r + "/CountRate"}).get_number();
  lambda_ = set.find({r + "/Lambda"}).get_number();
  spill_lambda_ = set.find({r + "/SpillLambda"}).get_number();
  dead_ = set.find({r + "/DeadTime"}).get_number() * 0.01;

  TimeBasePlugin tbs;
  tbs.settings(set.find({tbs.plugin_name()}));
  event_definition_ = EventModel();
  event_definition_.timebase = tbs.timebase();

  uint16_t val_count_ = std::max(int(set.find({r + "/ValueCount"}).get_number()), 1);
  if (val_defs_.size() != val_count_)
    val_defs_.resize(val_count_);

  for (Setting v : set.branches)
  {
    if (v.id() != "Value")
      continue;
    auto indices = v.indices();
    if (!indices.size())
      continue;
    size_t idx = *indices.begin();
//    DBG( "Write idx " << idx;
    if (idx >= val_defs_.size())
      continue;
    val_defs_[idx].settings(v);
  }

  for (size_t i = 0; i < val_defs_.size(); ++i)
    val_defs_[i].define(event_definition_);
}

void MockProducer::boot()
{
  if (!(status_ & ProducerStatus::can_boot))
  {
    WARN("<MockProducer> Cannot boot MockProducer. Failed flag check (can_boot == 0)");
    return;
  }

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;

  INFO("<MockProducer> Booting rate = {} cps", count_rate_);
//        << " with peak at " << peak_center_ << "%   stdev=" << peak_spread_;


  clock_ = 0;
  status_ = ProducerStatus::loaded | ProducerStatus::booted | ProducerStatus::can_run;
}

void MockProducer::die()
{
//  INFO( "<MockProducer> Shutting down";
  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

void MockProducer::worker_run(SpillQueue spill_queue)
{
  DBG("<MockProducer> Starting run   "
      "  timebase {} ns   init_rate = {} cps   lambda = {}",
      event_definition_.timebase.debug(), count_rate_, lambda_);

  Timer timer(true);

  spill_queue->enqueue(get_spill(Spill::Type::start, timer.s()));
  while (!terminate_.load())
  {
    spill_queue->enqueue(get_spill(Spill::Type::running, timer.s()));
    Timer::wait_s(spill_interval_);

    double seconds = timer.s();
    double overshoot = ((event_definition_.timebase.to_sec(clock_) - seconds) / seconds * 100.0);
    if (overshoot > 0)
      DBG("<MockProducer> Native clock overshoot {}%", overshoot);
  }
  spill_queue->enqueue(get_spill(Spill::Type::stop, timer.s()));
}

void MockProducer::add_hit(Spill& spill, uint64_t time)
{
  auto& e = spill.events.last();
  e.set_time(time);
  for (size_t i = 0; i < val_defs_.size(); ++i)
    val_defs_[i].generate(i, e);
  ++spill.events;
}

SpillPtr MockProducer::get_spill(Spill::Type t, double seconds)
{
  SpillPtr spill = std::make_shared<Spill>(stream_id_, t);

  recent_pulse_time_ = clock_ = event_definition_.timebase.to_native(seconds * pow(10, 9));

  if (t == Spill::Type::running)
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
