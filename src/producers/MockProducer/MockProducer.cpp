#include "MockProducer.h"
#include "custom_timer.h"

#include "custom_logger.h"

MockProducer::MockProducer()
{
  std::string mp {"MockProducer/"};

  SettingMeta si(mp + "SpillInterval", SettingType::integer, "Interval between spills");
  si.set_val("min", 1);
  si.set_val("max", 1000000);
  si.set_val("units", "s");
  add_definition(si);

  SettingMeta res(mp + "Resolution", SettingType::integer, "Resolution");
  res.set_val("min", 4);
  res.set_val("max", 16);
  res.set_val("units", "bits");
  add_definition(res);

  SettingMeta cr(mp + "CountRate", SettingType::floating, "Event rate");
  cr.set_val("min", 1);
  cr.set_val("units", "cps");
  add_definition(cr);

  SettingMeta tm(mp + "TimebaseMult", SettingType::integer, "Timebase multiplier");
  tm.set_val("min", 1);
  tm.set_val("units", "ns");
  add_definition(tm);

  SettingMeta td(mp + "TimebaseDiv", SettingType::integer, "Timebase divider");
  td.set_val("min", 1);
  td.set_val("units", "1/ns");
  add_definition(td);

  SettingMeta dt(mp + "DeadTime", SettingType::floating, "Dead time (% of real time)");
  dt.set_val("min", 0);
  dt.set_val("max", 100);
  dt.set_val("step", 0.01);
  add_definition(dt);

  SettingMeta lambda(mp + "Lambda", SettingType::floating, "Decay constant (λ)");
  lambda.set_val("min", 0);
  lambda.set_val("step", 0.01);
  add_definition(lambda);

  SettingMeta lambda2(mp + "SpillLambda", SettingType::floating, "Decay constant (λ) per spill");
  lambda2.set_val("min", 0);
  lambda2.set_val("max", 100);
  lambda2.set_val("units", "%");
  lambda2.set_val("step", 0.1);
  add_definition(lambda2);

  SettingMeta vc(mp + "ValueCount", SettingType::integer, "Value count");
  vc.set_flag("preset");
  vc.set_val("min", 1);
  vc.set_val("max", 16);
  add_definition(vc);


  SettingMeta valname(mp + "Value/Name", SettingType::text, "Value name");
  valname.set_flag("preset");
  add_definition(valname);

  SettingMeta pc(mp + "Value/PeakCenter", SettingType::floating, "Peak center (% resolution)");
  pc.set_flag("preset");
  pc.set_val("min", 0);
  pc.set_val("max", 100);
  pc.set_val("step", 0.1);
  add_definition(pc);

  SettingMeta ps(mp + "Value/PeakSpread", SettingType::floating, "Peak spread (stddev)");
  ps.set_flag("preset");
  ps.set_val("min", 0);
  ps.set_val("step", 0.01);
  add_definition(ps);

  SettingMeta val(mp + "Value", SettingType::stem);
  val.set_enum(0, mp + "Value/Name");
  val.set_enum(1, mp + "Value/PeakCenter");
  val.set_enum(2, mp + "Value/PeakSpread");
  add_definition(val);



  SettingMeta root("MockProducer", SettingType::stem);
  root.set_flag("producer");
  root.set_enum(0, mp + "SpillInterval");
  root.set_enum(1, mp + "Resolution");
  root.set_enum(2, mp + "CountRate");
  root.set_enum(3, mp + "DeadTime");
  root.set_enum(5, mp + "TimebaseMult");
  root.set_enum(6, mp + "TimebaseDiv");
  root.set_enum(7, mp + "Lambda");
  root.set_enum(8, mp + "SpillLambda");
  root.set_enum(9, mp + "ValueCount");
  add_definition(root);

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

MockProducer::~MockProducer()
{
  daq_stop();
  if (runner_ != nullptr)
  {
    runner_->detach();
    delete runner_;
  }
  die();
}

bool MockProducer::daq_start(SpillQueue out_queue)
{
  if (run_status_.load() > 0)
    return false;

  run_status_.store(1);

  if (runner_ != nullptr)
    delete runner_;

  runner_ = new std::thread(&worker_run, this, out_queue);

  return true;
}

bool MockProducer::daq_stop()
{
  if (run_status_.load() == 0)
    return false;

  run_status_.store(2);

  if ((runner_ != nullptr) && runner_->joinable())
  {
    runner_->join();
    delete runner_;
    runner_ = nullptr;
  }

  wait_ms(500);

  run_status_.store(0);
  return true;
}

bool MockProducer::daq_running()
{
  if (run_status_.load() == 3)
    daq_stop();
  return (run_status_.load() > 0);
}

void MockProducer::read_settings_bulk(Setting &set) const
{
  if (set.id() != device_name())
    return;
  set.enrich(setting_definitions_, true);
  set.enable_if_flag(!(status_ & booted), "preset");

  set.set(Setting::integer("MockProducer/SpillInterval", spill_interval_));
  set.set(Setting::integer("MockProducer/Resolution", bits_));
  set.set(Setting::floating("MockProducer/CountRate", count_rate_));
  set.set(Setting::floating("MockProducer/DeadTime", dead_*100));
  set.set(Setting::integer("MockProducer/TimebaseMult", model_hit.timebase.multiplier()));
  set.set(Setting::integer("MockProducer/TimebaseDiv", model_hit.timebase.divider()));
  set.set(Setting::floating("MockProducer/Lambda", lambda_));
  set.set(Setting::floating("MockProducer/SpillLambda", spill_lambda_));
  set.set(Setting::integer("MockProducer/ValueCount", integer_t(val_count_)));

  while (set.branches.has_a(Setting({"MockProducer/Value", SettingType::stem})))
    set.branches.remove_a(Setting({"MockProducer/Value", SettingType::stem}));

  for (int i=0; i < int(val_count_); ++i)
  {
    Setting v = get_rich_setting("MockProducer/Value");
    v.set_indices({i});
    v.branches = get_rich_setting("MockProducer/Value").branches;
    v.set(Setting::text("MockProducer/Value/Name", vnames_[i]));
    v.set(Setting::floating("MockProducer/Value/PeakCenter", centers_[i]*100));
    v.set(Setting::floating("MockProducer/Value/PeakSpread", spreads_[i]));
    for (auto& vv : v.branches)
      vv.set_indices({i});
    set.branches.add_a(v);
  }
}


void MockProducer::write_settings_bulk(const Setting &settings)
{
  if (settings.id() != device_name())
    return;
  auto set = settings;
  set.enrich(setting_definitions_, true);
  set.enable_if_flag(!(status_ & booted), "preset");

  spill_interval_ = set.find({"MockProducer/SpillInterval"}).get_number();
  bits_ = set.find({"MockProducer/Resolution"}).get_number();
  count_rate_ = set.find({"MockProducer/CountRate"}).get_number();
  lambda_ = set.find({"MockProducer/Lambda"}).get_number();
  spill_lambda_ = set.find({"MockProducer/SpillLambda"}).get_number();
  dead_ = set.find({"MockProducer/DeadTime"}).get_number() * 0.01;

  val_count_ = set.find({"MockProducer/ValueCount"}).get_number();
  if ((val_count_ < 1) || (val_count_ > 16))
    val_count_ = 1;

  if (vnames_.size() != val_count_)
    vnames_.resize(val_count_);
  if (centers_.size() != val_count_)
    centers_.resize(val_count_);
  if (spreads_.size() != val_count_)
    spreads_.resize(val_count_);
  if (dists_.size() != val_count_)
    dists_.resize(val_count_);

  model_hit = EventModel();
  model_hit.timebase = TimeBase(set.find({"MockProducer/TimebaseMult"}).get_number(),
                                set.find({"MockProducer/TimebaseDiv"}).get_number());

  for (Setting v : set.branches)
  {
    if (v.id() != "MockProducer/Value")
      continue;
    auto indices = v.indices();
    if (!indices.size())
      continue;
    size_t idx = *indices.begin();
//    DBG << "Write idx " << idx;
    if (idx >= val_count_)
      continue;
    centers_[idx] = v.find({"MockProducer/Value/PeakCenter"}).get_number() * 0.01;
    spreads_[idx] = v.find({"MockProducer/Value/PeakSpread"}).get_number();
    vnames_[idx] = v.find({"MockProducer/Value/Name"}).get_text();
  }

  for (size_t i=0; i < val_count_; ++i)
  {
    model_hit.add_value(vnames_[i], bits_);
    //  model_hit.add_trace("trace", {200});
    dists_[i] =
        std::normal_distribution<double>(centers_[i] * resolution_,
                                         spreads_[i]);
    DBG << "Adding " << i << " as " << vnames_[i]
           << " at center=" << centers_[i]
              << " spread=" << spreads_[i];
  }

  resolution_ = pow(2, uint32_t(bits_));

  // account for dead-time more realistically?
  event_interval_ = model_hit.timebase.to_native(spill_interval_ * pow(10,9))
      / count_rate_;
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
        << " resolution=" << resolution_ << " rate=" << count_rate_ << "cps";
//        << " with peak at " << peak_center_ << "%   stdev=" << peak_spread_;


  clock_ = 0;
  status_ = ProducerStatus::loaded | ProducerStatus::booted | ProducerStatus::can_run;
}

void MockProducer::die()
{
  INFO << "<MockProducer> Shutting down";
  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

void MockProducer::worker_run(MockProducer* callback,
                              SpillQueue spill_queue)
{
  DBG << "<MockProducer> Starting run   "
      << "  timebase " << callback->model_hit.timebase.debug() << "ns"
      << "  init_rate=" << callback->count_rate_ << "cps"
      << "  event_interval=" << callback->event_interval_ << "ticks"
      << "  lambda=" << callback->lambda_;

  CustomTimer timer(true);

  spill_queue->enqueue(callback->get_spill(StatusType::start, timer.s()));
  while (callback->run_status_.load() != 2)
  {
    wait_ms(callback->spill_interval_ * 1000);
    spill_queue->enqueue(callback->get_spill(StatusType::running, timer.s()));
  }
  spill_queue->enqueue(callback->get_spill(StatusType::stop, timer.s()));

  callback->run_status_.store(3);
}

void MockProducer::add_hit(Spill& spill)
{
  int16_t chan0 {0};

  Event h(chan0, model_hit);
  h.set_native_time(clock_);
  for (size_t i=0; i < dists_.size(); ++i)
    h.set_value(i, generate(i));

//  make_trace(h, 1000);

  spill.events.push_back(h);
  clock_ += event_interval_ + 1;
}

uint16_t MockProducer::generate(size_t i)
{
  if (i >= dists_.size())
    return 0;
  double v = dists_[i](gen_);
  v= std::max(std::min(v, double(resolution_)), 0.0);
  return std::round(v);
}

void MockProducer::make_trace(Event& h, uint16_t baseline)
{
  uint16_t en = h.value(0).val(h.value(0).bits());
  std::vector<uint16_t> trc(h.trace(0).size(), baseline);
  size_t start = double(trc.size()) * 0.1;
  double slope1 = double(en) / double(start);
  double slope2 = - double(en) / double(trc.size() * 10);
  for (size_t i = 0; i < start; ++i)
    trc[start+i] += i*slope1;
  for (size_t i = start*2; i < trc.size(); ++i)
    trc[i] += en + (i - 2*start) * slope2;
  for (size_t i=0; i < trc.size(); ++i)
    trc[i] += (rand() % baseline) / 5 - baseline/10;
  h.set_trace(0, trc);
}

Spill* MockProducer::get_spill(StatusType t, double seconds)
{
  int16_t chan0 {0};

  Spill* spill = new Spill();

  recent_pulse_time_ = clock_;

//  DBG << "----pulse--- " << clock_;

  if (t == StatusType::running)
  {
    double rate = count_rate_;

    if (lambda_)
    {
      double frac = exp(0.0 - lambda_ * seconds);
      rate *= frac;
      DBG << "<MockProducer> s=" << seconds << "  frac=" << frac << "  rate=" << rate;
    }

//    DBG << "Will make hits for " << (rate * spill_interval_);

    std::uniform_real_distribution<> dis(0, 1);
    uint32_t tothits = (rate * spill_interval_);
    for (uint32_t i=0; i< tothits; i++)
    {
      if (spill_lambda_ < 100)
      {
        double diff = (spill_lambda_ * 0.01)  +
            (1.0 - (spill_lambda_ * 0.01)) *
            (double(tothits - i) / double(tothits));
        if (dis(gen_) < diff)
        {
          add_hit(*spill);
//          DBG << "+ " << clock_;
        }
//        else
//          DBG << "- " << clock_;
      }
      else
        add_hit(*spill);
    }

//    DBG << "Added " << spill->events.size() << " events to spill";

  }

  spill->stats[chan0] = get_status(chan0, t);

  return spill;
}

Status MockProducer::get_status(int16_t chan, StatusType t)
{
  Status status;
  status.set_type(t);
  status.set_channel(chan);
  status.set_model(model_hit);
  status.set_time(boost::posix_time::microsec_clock::universal_time());

  double duration = clock_;
  double duration_live = duration * (1.0 - dead_);
  double duration_trigger = duration * (1.0 - 0.5 * dead_);

  status.set_value("native_time", duration);
  status.set_value("live_time", duration_live);
  status.set_value("live_trigger", duration_trigger);
  status.set_value("pulse_time", double(recent_pulse_time_));

  return status;
}
