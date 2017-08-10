#include "kafka_producer.h"
#include "custom_timer.h"

#include "custom_logger.h"

//#include "producer_factory.h"
//static ProducerRegistrar<KafkaProducer> registrar("KafkaProducer");

KafkaProducer::KafkaProducer()
{
  std::string mp {"KafkaProducer/"};

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

  SettingMeta tm(mp + "TimebaseMult", SettingType::integer, "Timebase multiplier");
  tm.set_val("min", 1);
  tm.set_val("units", "ns");
  add_definition(tm);

  SettingMeta td(mp + "TimebaseDiv", SettingType::integer, "Timebase divider");
  td.set_val("min", 1);
  td.set_val("units", "1/ns");
  add_definition(td);

  SettingMeta broker(mp + "Broker", SettingType::text, "Kafka Broker URL");
  add_definition(broker);

  SettingMeta topic(mp + "Topic", SettingType::text, "Kafka Topic");
  add_definition(topic);

  SettingMeta root("KafkaProducer", SettingType::stem);
  root.set_enum(0, mp + "SpillInterval");
  root.set_enum(1, mp + "Resolution");
  root.set_enum(2, mp + "TimebaseMult");
  root.set_enum(3, mp + "TimebaseDiv");
//  root.set_enum(9, mp + "Value");

  add_definition(root);

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

KafkaProducer::~KafkaProducer()
{
  daq_stop();
  if (runner_ != nullptr)
  {
    runner_->detach();
    delete runner_;
  }
  die();
}

bool KafkaProducer::daq_start(SpillQueue out_queue)
{
  if (run_status_.load() > 0)
    return false;

  run_status_.store(1);

  if (runner_ != nullptr)
    delete runner_;

  runner_ = new boost::thread(&worker_run, this, out_queue);

  return true;
}

bool KafkaProducer::daq_stop()
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

bool KafkaProducer::daq_running()
{
  if (run_status_.load() == 3)
    daq_stop();
  return (run_status_.load() > 0);
}

void KafkaProducer::read_settings_bulk(Setting &set) const
{
  if (set.id() != device_name())
    return;
  set.set(Setting::integer("KafkaProducer/SpillInterval", spill_interval_));
  set.set(Setting::integer("KafkaProducer/Resolution", bits_));
  set.set(Setting::integer("KafkaProducer/TimebaseMult", model_hit.timebase.multiplier()));
  set.set(Setting::integer("KafkaProducer/TimebaseDiv", model_hit.timebase.divider()));
  set.set(Setting::text("KafkaProducer/Broker", broker_));
  set.set(Setting::text("KafkaProducer/Topic", topic_));
}


void KafkaProducer::write_settings_bulk(Setting &set)
{
  if (set.id() != device_name())
    return;

  set.enrich(setting_definitions_, true);

  spill_interval_ = set.find({"KafkaProducer/SpillInterval"}).get_number();
  bits_ = set.find({"KafkaProducer/Resolution"}).get_number();

  model_hit = EventModel();
  model_hit.timebase = TimeBase(set.find({"KafkaProducer/TimebaseMult"}).get_number(),
                                set.find({"KafkaProducer/TimebaseDiv"}).get_number());


  broker_ = set.find({"KafkaProducer/Broker"}).get_text();
  topic_ = set.find({"KafkaProducer/Topic"}).get_text();
}

void KafkaProducer::boot()
{
  if (!(status_ & ProducerStatus::can_boot))
  {
    WARN << "<KafkaProducer> Cannot boot KafkaProducer. Failed flag check (can_boot == 0)";
    return;
  }

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;

  INFO << "<KafkaProducer> Booting mock producer";

  std::string error_str;

  auto conf = std::unique_ptr<RdKafka::Conf>(
        RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));

  if (!conf.get())
  {
    ERR << "Unable to created global Conf object";
    die();
    return;
  }

  conf->set("metadata.broker.list", broker_, error_str);
  conf->set("message.max.bytes", "10000000", error_str);
  conf->set("fetch.message.max.bytes", "10000000", error_str);
  conf->set("replica.fetch.max.bytes", "10000000", error_str);

  conf->set("group.id", "nexus_stream_consumer", error_str);
  conf->set("enable.auto.commit", "false", error_str);
  conf->set("enable.auto.offset.store", "false", error_str);
  conf->set("offset.store.method", "none", error_str);
  conf->set("auto.offset.reset", "largest", error_str);

  consumer_ = std::unique_ptr<RdKafka::KafkaConsumer>(
        RdKafka::KafkaConsumer::create(conf.get(), error_str));
  if (!consumer_.get())
  {
    ERR << "Failed to create consumer: " << error_str;
    die();
    return;
  }

  INFO << "Created consumer " << consumer_->name();

  // Start consumer for topic+partition at start offset
  std::vector<std::string> topics = {topic_};
  RdKafka::ErrorCode resp = consumer_->subscribe(topics);
  if (resp != RdKafka::ERR_NO_ERROR)
  {
    ERR << "Failed to start consumer: " << RdKafka::err2str(resp);
    die();
    return;
  }


  clock_ = 0;
  status_ = ProducerStatus::loaded | ProducerStatus::booted | ProducerStatus::can_run;
}

void KafkaProducer::die()
{
  INFO << "<KafkaProducer> Die mock producer";
  if (consumer_)
  {
    consumer_->close();
    // Wait for RdKafka to decommission, avoids complaints of memory leak from
    // valgrind etc.
    RdKafka::wait_destroyed(5000);
  }
  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

void KafkaProducer::worker_run(KafkaProducer* callback,
                              SpillQueue spill_queue)
{
  DBG << "<KafkaProducer> Starting run   "
      << "  timebase " << callback->model_hit.timebase.debug() << "ns";

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

void KafkaProducer::add_hit(Spill& spill)
{
  int16_t chan0 {0};

  Event h(chan0, model_hit);
  h.set_native_time(clock_);

  spill.events.push_back(h);
  clock_ += 1;
}

Spill* KafkaProducer::get_spill(StatusType t, double seconds)
{
  int16_t chan0 {0};

  Spill* spill = new Spill();

  if (t == StatusType::running)
  {
//    DBG << "Will make hits for " << (rate * spill_interval_);

    for (uint32_t i=0; i< (spill_interval_); i++)
      add_hit(*spill);

//    DBG << "Added " << spill->events.size() << " events to spill";

  }

  spill->stats[chan0] = get_status(chan0, t);

  return spill;
}

Status KafkaProducer::get_status(int16_t chan, StatusType t)
{
  Status status;
  status.set_type(t);
  status.set_channel(chan);
  status.set_model(model_hit);
  status.set_time(boost::posix_time::microsec_clock::universal_time());

  double duration = clock_;

  status.set_value("native_time", duration);

  return status;
}
