#include "mock_producer.h"
#include "custom_timer.h"

#include "custom_logger.h"

//#include "producer_factory.h"
//static ProducerRegistrar<MockProducer> registrar("MockProducer");

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

  SettingMeta root("MockProducer", SettingType::stem);
  root.set_enum(0, mp + "SpillInterval");
  root.set_enum(1, mp + "Resolution");
  root.set_enum(2, mp + "CountRate");
  root.set_enum(3, mp + "DeadTime");
  root.set_enum(5, mp + "TimebaseMult");
  root.set_enum(6, mp + "TimebaseDiv");
  root.set_enum(7, mp + "Lambda");
  root.set_enum(8, mp + "ValName1");
  root.set_enum(9, mp + "ValName2");
  root.set_enum(10, mp + "PeakCenter");
  root.set_enum(11, mp + "PeakSpread");

  add_dummy_settings();
  root.set_enum(1000, "MockProducer/DummySettings");
  add_definition(root);

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

void MockProducer::add_dummy_settings()
{
  std::string r {"MockProducer/DummySettings"};

  SettingMeta a1(r + "/IntUnbounded", SettingType::integer);
  add_definition(a1);

  SettingMeta a2(r + "/IntLB", SettingType::integer);
  a2.set_val("min", 0);
  add_definition(a2);

  SettingMeta a3(r + "/IntUB", SettingType::integer);
  a3.set_val("max", 0);
  add_definition(a3);

  SettingMeta a4(r + "/IntBounded", SettingType::integer);
  a4.set_val("min", 2);
  a4.set_val("max", 4);
  add_definition(a4);



  SettingMeta a5(r + "/FloatUnbounded", SettingType::floating);
  a5.set_val("step", 0.05);
  add_definition(a5);

  SettingMeta a6(r + "/FloatLB", SettingType::floating);
  a6.set_val("min", 0);
  a6.set_val("step", 0.2);
  add_definition(a6);

  SettingMeta a7(r + "/FloatUB", SettingType::floating);
  a7.set_val("max", 0);
  a7.set_val("step", 0.5);
  add_definition(a7);

  SettingMeta a8(r + "/FloatBounded", SettingType::floating);
  a8.set_val("min", 2);
  a8.set_val("max", 4);
  a8.set_val("step", 0.1);
  add_definition(a8);



  SettingMeta a9(r + "/PreciseUnbounded", SettingType::precise);
  a9.set_val("step", 0.05);
  add_definition(a9);

  SettingMeta a10(r + "/PreciseLB", SettingType::precise);
  a10.set_val("min", 0);
  a10.set_val("step", 0.2);
  add_definition(a10);

  SettingMeta a11(r + "/PreciseUB", SettingType::precise);
  a11.set_val("max", 0);
  a11.set_val("step", 0.5);
  add_definition(a11);

  SettingMeta a12(r + "/PreciseBounded", SettingType::precise);
  a12.set_val("min", 2);
  a12.set_val("max", 4);
  a12.set_val("step", 0.1);
  add_definition(a12);


  SettingMeta a13(r + "/Time", SettingType::time);
  add_definition(a13);

  SettingMeta a14(r + "/Duration", SettingType::duration);
  add_definition(a14);

  SettingMeta a15(r + "/Pattern", SettingType::pattern);
  a15.set_val("chans", 6);
  add_definition(a15);



  SettingMeta a16(r + "/Boolean", SettingType::boolean);
  add_definition(a16);

  SettingMeta a17(r + "/text", SettingType::text);
  add_definition(a17);

  SettingMeta a18(r + "/Color", SettingType::color);
  add_definition(a18);

  SettingMeta a19(r + "/File", SettingType::file);
  a19.set_val("wildcards", "Bash file (*.sh)");
  add_definition(a19);

  SettingMeta a20(r + "/Directory", SettingType::dir);
  add_definition(a20);

  SettingMeta a21(r + "/Detector", SettingType::detector);
  add_definition(a21);


  SettingMeta a22(r + "/Command", SettingType::command);
  add_definition(a22);

  SettingMeta a23(r + "/Menu", SettingType::menu);
  a23.set_enum(0, "a");
  a23.set_enum(1, "b");
  a23.set_enum(2, "c");
  add_definition(a23);


  SettingMeta a24(r + "/Binary", SettingType::binary);
  a24.set_val("bits", 16);
  a24.set_enum(0, "bit0");
  a24.set_enum(1, "bit1");
  a24.set_enum(3, "bit3");
  a24.set_enum(4, r + "/Binary/bit4");
  a24.set_enum(8, r + "/Binary/bit8");
  add_definition(a24);
  SettingMeta a24b4(r + "/Binary/bit4", SettingType::menu);
  a24b4.set_enum(0, "c0");
  a24b4.set_enum(1, "c1");
  a24b4.set_enum(2, "c2");
  a24b4.set_enum(3, "c3");
  a24b4.set_val("name", "name of a24b4");
  add_definition(a24b4);
  SettingMeta a24b8(r + "/Binary/bit8", SettingType::integer);
  a24b8.set_val("bits", 8);
  a24b8.set_val("name", "name of a24b8");
  add_definition(a24b8);


  SettingMeta a25(r + "/Indicator", SettingType::indicator);
  a25.set_enum(0, r + "/Indicator/a");
  a25.set_enum(1, r + "/Indicator/b");
  a25.set_enum(2, r + "/Indicator/c");
  add_definition(a25);
  SettingMeta a26(r + "/Indicator/a", SettingType::text);
  a26.set_val("name", "A");
  a26.set_val("color", "#FF0000");
  add_definition(a26);
  SettingMeta a27(r + "/Indicator/b", SettingType::text);
  a27.set_val("name", "B");
  a27.set_val("color", "#00FF00");
  add_definition(a27);
  SettingMeta a28(r + "/Indicator/c", SettingType::text);
  a28.set_val("name", "C");
  a28.set_val("color", "#0000FF");
  add_definition(a28);

  SettingMeta root(r, SettingType::stem);
  root.set_enum(1, a1.id());
  root.set_enum(2, a2.id());
  root.set_enum(3, a3.id());
  root.set_enum(4, a4.id());
  root.set_enum(5, a5.id());
  root.set_enum(6, a6.id());
  root.set_enum(7, a7.id());
  root.set_enum(8, a8.id());
  root.set_enum(9, a9.id());
  root.set_enum(10, a10.id());
  root.set_enum(11, a11.id());
  root.set_enum(12, a12.id());
  root.set_enum(13, a13.id());
  root.set_enum(14, a14.id());
  root.set_enum(15, a15.id());
  root.set_enum(16, a16.id());
  root.set_enum(17, a17.id());
  root.set_enum(18, a18.id());
  root.set_enum(19, a19.id());
  root.set_enum(20, a20.id());
  root.set_enum(21, a21.id());
  root.set_enum(22, a22.id());
  root.set_enum(23, a23.id());
  root.set_enum(24, a24.id());
  root.set_enum(25, a25.id());
  add_definition(root);

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

  set.set(Setting::indicator("MockProducer/DummySettings/Indicator", dummy_selection_));
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

  dummy_selection_ = set.find({"MockProducer/DummySettings/Menu"}).selection();

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
