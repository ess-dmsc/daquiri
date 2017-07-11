#include "mock_producer.h"
#include "custom_timer.h"

#include "custom_logger.h"

//#include "producer_factory.h"
//static ProducerRegistrar<MockProducer> registrar("MockProducer");

MockProducer::MockProducer()
{
  status_ = ProducerStatus::loaded;// | ProducerStatus::can_boot;
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
      else if (q.id() == "MockProducer/ScaleRate")
        q.set_number(count_rate_);
      else if (q.id() == "MockProducer/CoincThresh")
        q.set_number(event_interval_);
      else if (q.id() == "MockProducer/TimebaseMult")
        q.set_number(model_hit.timebase.multiplier());
      else if (q.id() == "MockProducer/TimebaseDiv")
        q.set_number(model_hit.timebase.divider());
      else if (q.id() == "MockProducer/Lambda")
        q.set_number(lambda_);
    }
  }
}


void MockProducer::write_settings_bulk(Setting &set)
{
  if (set.id() != device_name())
    return;

  double timebase_multiplier = model_hit.timebase.multiplier();
  double timebase_divider = model_hit.timebase.divider();

  for (auto &q : set.branches)
  {
    if (q.id() == "MockProducer/SpillInterval")
      spill_interval_ = q.get_number();
    else if (q.id() == "MockProducer/Resolution")
      bits_ = q.get_number();
    else if (q.id() == "MockProducer/CoincThresh")
      event_interval_ = q.get_number();
    else if (q.id() == "MockProducer/TimebaseMult")
      timebase_multiplier = q.get_number();
    else if (q.id() == "MockProducer/TimebaseDiv")
      timebase_divider = q.get_number();
    else if (q.id() == "MockProducer/ScaleRate")
      count_rate_ = q.get_number();
    else if (q.id() == "MockProducer/Lambda")
      lambda_ = q.get_number();
  }

  model_hit = EventModel();
  model_hit.timebase = TimeBase(timebase_multiplier, timebase_divider);
  model_hit.add_value("energy", 16);
  model_hit.add_value("junk", 16);
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

  if (lab_time == 0.0)
  {
    WARN << "<MockProducer> Lab time = 0. Cannot create simulation.";
    return false;
  }

  resolution_ = pow(2, bits_);

  LINFO << "<MockProducer> Building matrix for simulation from "
       << " resolution=" << resolution_ << " rate=" << count_rate_ << "cps";
  std::vector<double> distribution(resolution_, 0.0);   //optimize somehow

//  for (auto it : *spec_list)
//    distribution[(it.first[0] >> adjust_bits) * resolution_
//        + (it.first[1] >> adjust_bits)]
//        =  static_cast<double>(it.second) / totevts;

  dist_ = boost::random::discrete_distribution<>(distribution);

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

  double   rate = callback->count_rate_;
  double   lambda = callback->lambda_;
  Status moving_stats,
      one_run = callback->getBlock(callback->spill_interval_ * 0.99);

  Spill one_spill;

  DBG << "<MockProducer> Start run   "
      << "  timebase " << callback->model_hit.timebase.debug() << "ns"
      << "  init_rate=" << rate << " cps"
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
    uint64_t rate = rate * exp(0.0 - lambda * timer.s());
    boost::this_thread::sleep(boost::posix_time::seconds(callback->spill_interval_));

    DBG << "<MockProducer> s=" << timer.s() << " exp=" << exp(0.0 - lambda * timer.s())
        << "  current rate = " << rate;

    one_spill = Spill();

    for (uint32_t i=0; i< (rate * callback->spill_interval_); i++)
    {
      if (callback->resolution_ > 0)
      {
        uint64_t newpoint = callback->dist_(callback->gen);
        int32_t en1 = newpoint / callback->resolution_;

        callback->push_hit(one_spill, en1);
      }
    }

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

void MockProducer::push_hit(Spill& one_spill, uint16_t en1)
{
  if (en1 > 0)
  {
//    Event h(chan0_, model_hit);
//    h.set_native_time(clock_);
//    h.set_value(0, round(en1 * gain0_ * 0.01));
//    h.set_value(1, rand() % 100);
//    make_trace(h, 1000);
//    one_spill.events.push_back(h);
  }

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
