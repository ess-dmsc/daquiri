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

  SettingMeta ei(mp + "EventInterval", SettingType::integer, "Event interval");
  ei.set_val("min", 1);
  ei.set_val("units", "ticks");
  add_definition(ei);

  SettingMeta tm(mp + "TimebaseMult", SettingType::integer, "Timebase multiplier");
  tm.set_val("min", 1);
  tm.set_val("units", "ns");
  add_definition(tm);

  SettingMeta td(mp + "TimebaseDiv", SettingType::integer, "Timebase divider");
  td.set_val("min", 1);
  td.set_val("units", "1/ns");
  add_definition(td);

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
  pc.set_val("min", 100);
  pc.set_val("step", 0.1);
  add_definition(pc);

  SettingMeta ps(mp + "PeakSpread", SettingType::floating, "Peak spread (stddev)");
  ps.set_val("min", 0);
  ps.set_val("step", 0.01);
  add_definition(ps);

  root.set_enum(0, mp + "SpillInterval");
  root.set_enum(1, mp + "Resolution");
  root.set_enum(2, mp + "CountRate");
  root.set_enum(3, mp + "EventInterval");
  root.set_enum(4, mp + "TimebaseMult");
  root.set_enum(5, mp + "TimebaseDiv");
  root.set_enum(6, mp + "Lambda");
  root.set_enum(7, mp + "ValName1");
  root.set_enum(8, mp + "ValName2");
  root.set_enum(9, mp + "PeakCenter");
  root.set_enum(10, mp + "PeakSpread");
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

bool MockProducer::daq_start(SynchronizedQueue<Spill*>* out_queue)
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
  if (set.id() == device_name())
  {
    for (auto &q : set.branches)
    {
      if (q.id() == "MockProducer/SpillInterval")
        q.set_number(spill_interval_);
      else if (q.id() == "MockProducer/Resolution")
        q.select(bits_);
      else if (q.id() == "MockProducer/CountRate")
        q.set_number(count_rate_);
      else if (q.id() == "MockProducer/EventInterval")
        q.set_number(event_interval_);
      else if (q.id() == "MockProducer/TimebaseMult")
        q.set_number(model_hit.timebase.multiplier());
      else if (q.id() == "MockProducer/TimebaseDiv")
        q.set_number(model_hit.timebase.divider());
      else if (q.id() == "MockProducer/Lambda")
        q.set_number(lambda_);
      else if (q.id() == "MockProducer/ValName1")
        q.set_text((model_hit.values.size() > 0) ? model_hit.value_names.at(0) : "v1");
      else if (q.id() == "MockProducer/ValName2")
        q.set_text((model_hit.values.size() > 1) ? model_hit.value_names.at(1) : "v2");
      else if (q.id() == "MockProducer/PeakCenter")
        q.set_number(peak_center_ * 100);
      else if (q.id() == "MockProducer/PeakSpread")
        q.set_number(peak_spread_);
    }
  }
}


void MockProducer::write_settings_bulk(Setting &set)
{
  if (set.id() != device_name())
    return;

  double timebase_multiplier = model_hit.timebase.multiplier();
  double timebase_divider = model_hit.timebase.divider();
  std::string vname1 {"v1"}, vname2 {"v2"};

  for (auto &q : set.branches)
  {
    if (q.id() == "MockProducer/SpillInterval")
      spill_interval_ = q.get_number();
    else if (q.id() == "MockProducer/Resolution")
      bits_ = q.get_number();
    else if (q.id() == "MockProducer/EventInterval")
      event_interval_ = q.get_number();
    else if (q.id() == "MockProducer/TimebaseMult")
      timebase_multiplier = q.get_number();
    else if (q.id() == "MockProducer/TimebaseDiv")
      timebase_divider = q.get_number();
    else if (q.id() == "MockProducer/CountRate")
      count_rate_ = q.get_number();
    else if (q.id() == "MockProducer/Lambda")
      lambda_ = q.get_number();
    else if (q.id() == "MockProducer/ValName1")
      vname1 = q.get_text();
    else if (q.id() == "MockProducer/ValName2")
      vname2 = q.get_text();
    else if (q.id() == "MockProducer/PeakCenter")
      peak_center_ = q.get_number() / 100.0;
    else if (q.id() == "MockProducer/PeakSpread")
      peak_spread_ = q.get_number();
  }

  model_hit = EventModel();
  model_hit.timebase = TimeBase(timebase_multiplier, timebase_divider);
  model_hit.add_value(vname1, bits_);
  model_hit.add_value(vname2, bits_);
  model_hit.add_trace("trace", {200});

  set.enrich(setting_definitions_);
}

bool MockProducer::boot()
{
  if (!(status_ & ProducerStatus::can_boot))
  {
    WARN << "<MockProducer> Cannot boot MockProducer. Failed flag check (can_boot == 0)";
    return false;
  }

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;

//  if (lab_time == 0.0)
//  {
//    WARN << "<MockProducer> Lab time = 0. Cannot create simulation.";
//    return false;
//  }

  resolution_ = pow(2, bits_);

  LINFO << "<MockProducer> Building matrix for simulation from "
       << " resolution=" << resolution_ << " rate=" << count_rate_ << "cps";

  dist_ = std::normal_distribution<double>(peak_center_, peak_spread_);

  clock_ = 0;
  lab_time = 0;
  live_time = 0;

  status_ = ProducerStatus::loaded | ProducerStatus::booted | ProducerStatus::can_run;
  return true;
}


void MockProducer::get_all_settings()
{
  if (status_ & ProducerStatus::booted)
  {

  }
}


void MockProducer::worker_run(MockProducer* callback,
                             SynchronizedQueue<Spill*>* spill_queue)
{
  bool timeout = false;

  double   lambda = callback->lambda_;
  Status moving_stats,
      one_run = callback->getBlock(callback->spill_interval_ * 0.99);

  Spill one_spill;

  DBG << "<MockProducer> Start run   "
      << "  timebase " << callback->model_hit.timebase.debug() << "ns"
      << "  init_rate=" << callback->count_rate_ << " cps"
      << "  lambda=" << lambda;

  one_spill = Spill();
  moving_stats.set_model(callback->model_hit);
  moving_stats.set_type(StatusType::start);
  moving_stats.set_time(boost::posix_time::microsec_clock::universal_time());

//  moving_stats.set_channel(callback->chan0_);
//  one_spill.stats[callback->chan0_] = moving_stats;

  spill_queue->enqueue(new Spill(one_spill));

  CustomTimer timer(true);
  while (!timeout)
  {
    double frac = exp(0.0 - lambda * timer.s());
    double rate = callback->count_rate_ * frac;
    boost::this_thread::sleep(boost::posix_time::seconds(callback->spill_interval_));

    DBG << "<MockProducer> s=" << timer.s()
        << "  frac=" << frac
        << "  rate=" << rate;

    one_spill = Spill();

    for (uint32_t i=0; i< (rate * callback->spill_interval_); i++)
      callback->add_hit(one_spill);

//    moving_stats = moving_stats + one_run;
    moving_stats.set_model(callback->model_hit);
    moving_stats.set_type(StatusType::running);
    moving_stats.set_time(boost::posix_time::microsec_clock::universal_time());

//    DBG << "pushing with model " << moving_stats.model_hit.to_string();

//    moving_stats.set_channel(callback->chan0_);
//    one_spill.stats[callback->chan0_] = moving_stats;

    spill_queue->enqueue(new Spill(one_spill));

    timeout = (callback->run_status_.load() == 2);
  }

  one_spill.events.clear();
  for (auto &q : one_spill.stats)
    q.second.set_type(StatusType::stop);

  spill_queue->enqueue(new Spill(one_spill));

  callback->run_status_.store(3);

  //  DBG << "<MockProducer> Stop run worker";
}

void MockProducer::add_hit(Spill& one_spill)
{
  if (resolution_ < 1)
    return;

  int16_t chan0 {0};

  double v1 = dist_(gen_) * resolution_;
  double v2 = dist_(gen_) * resolution_;

  Event h(chan0, model_hit);
  h.set_native_time(clock_);
  if (v1 > 0)
    h.set_value(0, round(v1));
  if (v2 > 0)
    h.set_value(1, round(v2));
  make_trace(h, 1000);
  one_spill.events.push_back(h);

  clock_ += event_interval_ + 1;
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

Spill MockProducer::get_spill()
{
  Spill one_spill;

  return one_spill;
}

Status MockProducer::getBlock(double duration)
{
  Status newBlock;

  double fraction;

  newBlock.set_value("native_time", duration);

  if (lab_time == 0.0)
    fraction = duration;
  else
    fraction = duration / lab_time;

  if (std::isfinite(live_time) && (live_time > 0))
  {
    newBlock.set_value("live_time", live_time);
    newBlock.set_value("live_trigger", live_time);
  }

  return newBlock;
}
