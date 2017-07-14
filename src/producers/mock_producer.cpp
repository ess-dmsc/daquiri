#include "mock_producer.h"
#include "custom_timer.h"

#include "custom_logger.h"

//#include "producer_factory.h"
//static ProducerRegistrar<MockProducer> registrar("MockProducer");

MockProducer::MockProducer()
{
  std::string mp {"MockProducer/"};

  SettingMeta root("MockProducer", SettingType::stem);

  SettingMeta si(mp + "SpillInterval", SettingType::integer, "Interval between spills");
  si.set_val("min", 1);
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

  SettingMeta val1(mp + "ValName1", SettingType::text, "Value name 1");
  add_definition(val1);

  SettingMeta val2(mp + "ValName2", SettingType::text, "Value name 2");
  add_definition(val2);

  SettingMeta pc(mp + "PeakCenter", SettingType::floating, "Peak center (% resolution)");
  pc.set_val("min", 0);
  pc.set_val("max", 100);
  pc.set_val("step", 0.1);
  add_definition(pc);

  SettingMeta ps(mp + "PeakSpread", SettingType::floating, "Peak spread (stddev)");
  ps.set_val("min", 0);
  ps.set_val("step", 0.01);
  add_definition(ps);

  root.set_enum(0, mp + "SpillInterval");
  root.set_enum(1, mp + "Resolution");
  root.set_enum(2, mp + "CountRate");
  root.set_enum(3, mp + "DeadTime");
  root.set_enum(4, mp + "EventInterval");
  root.set_enum(5, mp + "TimebaseMult");
  root.set_enum(6, mp + "TimebaseDiv");
  root.set_enum(7, mp + "Lambda");
  root.set_enum(8, mp + "ValName1");
  root.set_enum(9, mp + "ValName2");
  root.set_enum(10, mp + "PeakCenter");
  root.set_enum(11, mp + "PeakSpread");
  add_definition(root);

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

bool MockProducer::die()
{
  status_ = ProducerStatus::loaded;
  return true;
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

  runner_ = new boost::thread(&worker_run, this, out_queue);

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
  set.set(Setting::integer("MockProducer/SpillInterval", spill_interval_));
  set.set(Setting::integer("MockProducer/Resolution", bits_));
  set.set(Setting::floating("MockProducer/CountRate", count_rate_));
  set.set(Setting::floating("MockProducer/DeadTime", dead_*100));
  set.set(Setting::integer("MockProducer/TimebaseMult", model_hit.timebase.multiplier()));
  set.set(Setting::integer("MockProducer/TimebaseDiv", model_hit.timebase.divider()));
  set.set(Setting::floating("MockProducer/Lambda", lambda_));
  set.set(Setting::floating("MockProducer/PeakCenter", peak_center_*100));
  set.set(Setting::floating("MockProducer/PeakSpread", peak_spread_));
  set.set(Setting::text("MockProducer/ValName1", vname1));
  set.set(Setting::text("MockProducer/ValName2", vname2));
}


void MockProducer::write_settings_bulk(Setting &set)
{
  if (set.id() != device_name())
    return;

  set.enrich(setting_definitions_, true);

  spill_interval_ = set.find({"MockProducer/SpillInterval"}).get_number();
  bits_ = set.find({"MockProducer/Resolution"}).get_number();
  count_rate_ = set.find({"MockProducer/CountRate"}).get_number();
  lambda_ = set.find({"MockProducer/Lambda"}).get_number();
  dead_ = set.find({"MockProducer/DeadTime"}).get_number() * 0.01;
  peak_center_ = set.find({"MockProducer/PeakCenter"}).get_number() * 0.01;
  peak_spread_ = set.find({"MockProducer/PeakSpread"}).get_number();
  vname1 = set.find({"MockProducer/ValName1"}).get_text();
  vname2 = set.find({"MockProducer/ValName2"}).get_text();

  model_hit = EventModel();
  model_hit.timebase = TimeBase(set.find({"MockProducer/TimebaseMult"}).get_number(),
                                set.find({"MockProducer/TimebaseDiv"}).get_number());
  model_hit.add_value(vname1, bits_);
  model_hit.add_value(vname2, bits_);
//  model_hit.add_trace("trace", {200});

  resolution_ = pow(2, uint32_t(bits_));

  // account for dead-time more realistically?
  event_interval_ = model_hit.timebase.to_native(spill_interval_ * pow(10,9))
      / count_rate_;
}

bool MockProducer::boot()
{
  if (!(status_ & ProducerStatus::can_boot))
  {
    WARN << "<MockProducer> Cannot boot MockProducer. Failed flag check (can_boot == 0)";
    return false;
  }

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;

  LINFO << "<MockProducer> Booting mock producer"
        << " resolution=" << resolution_ << " rate=" << count_rate_ << "cps"
        << " with peak at " << peak_center_ << "%   stdev=" << peak_spread_;

  dist_ = std::normal_distribution<double>(peak_center_ * resolution_,
                                           peak_spread_);
  clock_ = 0;
  status_ = ProducerStatus::loaded | ProducerStatus::booted | ProducerStatus::can_run;
  return true;
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
    boost::this_thread::sleep(boost::posix_time::seconds(callback->spill_interval_));
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
  h.set_value(0, generate());
  h.set_value(1, generate());
//  make_trace(h, 1000);

  spill.events.push_back(h);
  clock_ += event_interval_ + 1;
}

uint16_t MockProducer::generate()
{
  double v = dist_(gen_);
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

    for (uint32_t i=0; i< (rate * spill_interval_); i++)
      add_hit(*spill);

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

  return status;
}
