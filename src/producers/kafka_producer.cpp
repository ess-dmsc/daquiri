#include "kafka_producer.h"
#include "custom_timer.h"

#include "custom_logger.h"

#include "ev42_events_generated.h"
#include "mon_efu_generated.h"

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

  SettingMeta tm(mp + "TimebaseMult", SettingType::integer, "Timebase multiplier");
  tm.set_val("min", 1);
  tm.set_val("units", "ns");
  add_definition(tm);

  SettingMeta td(mp + "TimebaseDiv", SettingType::integer, "Timebase divider");
  td.set_val("min", 1);
  td.set_val("units", "1/ns");
  add_definition(td);

  SettingMeta broker(mp + "KafkaBroker", SettingType::text, "Kafka Broker URL");
  add_definition(broker);

  SettingMeta topic(mp + "KafkaTopic", SettingType::text, "Kafka Topic");
  add_definition(topic);

  SettingMeta pi(mp + "KafkaPollInterval", SettingType::integer, "Kafka poll interval");
  pi.set_val("min", 1);
  pi.set_val("max", 1000000);
  pi.set_val("units", "ms");
  add_definition(pi);

  SettingMeta det_type(mp + "DetectorType", SettingType::text, "Detector type");
  add_definition(det_type);

  SettingMeta root("KafkaProducer", SettingType::stem);
  root.set_enum(0, mp + "SpillInterval");
  root.set_enum(1, mp + "Resolution");
  root.set_enum(2, mp + "TimebaseMult");
  root.set_enum(3, mp + "TimebaseDiv");
  root.set_enum(4, mp + "KafkaBroker");
  root.set_enum(5, mp + "KafkaTopic");
  root.set_enum(6, mp + "PollInterval");
  root.set_enum(7, mp + "DetectorType");

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
  set.set(Setting::integer("KafkaProducer/TimebaseMult", model_hit.timebase.multiplier()));
  set.set(Setting::integer("KafkaProducer/TimebaseDiv", model_hit.timebase.divider()));
  set.set(Setting::text("KafkaProducer/KafkaBroker", broker_));
  set.set(Setting::text("KafkaProducer/KafkaTopic", topic_));

  set.set(Setting::text("KafkaProducer/DetectorType", detector_type_));
}


void KafkaProducer::write_settings_bulk(Setting &set)
{
  if (set.id() != device_name())
    return;

  set.enrich(setting_definitions_, true);

  spill_interval_ = set.find({"KafkaProducer/SpillInterval"}).get_number();

  model_hit = EventModel();
  model_hit.timebase = TimeBase(set.find({"KafkaProducer/TimebaseMult"}).get_number(),
                                set.find({"KafkaProducer/TimebaseDiv"}).get_number());
  model_hit.add_value("pixid", 16);


  broker_ = set.find({"KafkaProducer/KafkaBroker"}).get_text();
  topic_ = set.find({"KafkaProducer/KafkaTopic"}).get_text();
  poll_interval_ = set.find({"KafkaProducer/KafkaPollInterval"}).get_number();

  detector_type_ = set.find({"KafkaProducer/DetectorType"}).get_text();
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

  //  conf->set("group.id", "nexus_stream_consumer", error_str);
  //  conf->set("enable.auto.commit", "false", error_str);
  //  conf->set("enable.auto.offset.store", "false", error_str);
  //  conf->set("offset.store.method", "none", error_str);
  //  conf->set("auto.offset.reset", "largest", error_str);

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
  RdKafka::ErrorCode resp = consumer_->subscribe({topic_});
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

  spill_queue->enqueue(callback->create_spill(StatusType::start));

  while (callback->run_status_.load() != 2)
  {
    auto spill = callback->listenForMessage();
    if (spill)
      spill_queue->enqueue(spill);
  }

  spill_queue->enqueue(callback->create_spill(StatusType::stop));

  callback->run_status_.store(3);
}

Spill* KafkaProducer::create_spill(StatusType t)
{
  int16_t chan0 {0};
  Spill* spill = new Spill();
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

Spill* KafkaProducer::listenForMessage()
{
  std::shared_ptr<RdKafka::Message> message
  {consumer_->consume(poll_interval_)};

  switch (message->err())
  {
  case RdKafka::ERR__TIMED_OUT:
    return nullptr;

  case RdKafka::ERR_NO_ERROR:
    //    msg_cnt++;
    //    msg_bytes += message->len();
    DBG << "Read msg at offset " << message->offset();
    RdKafka::MessageTimestamp ts;
    ts = message->timestamp();
    if (ts.type != RdKafka::MessageTimestamp::MSG_TIMESTAMP_NOT_AVAILABLE)
    {
      std::string tsname = "?";
      if (ts.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_CREATE_TIME)
        tsname = "create time";
      else if (ts.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_LOG_APPEND_TIME)
        tsname = "log append time";
      DBG << "Timestamp: " << tsname << " " << ts.timestamp;
    }

    if (message->key())
      DBG << "Key: " << *message->key();

    return messageConsume(message);

  case RdKafka::ERR__PARTITION_EOF:
    /* Last message */
    //    if (exit_eof && ++eof_cnt == partition_cnt)
    //      WARN << "%% EOF reached for all " << partition_cnt <<
    //                   " partition(s)";
    WARN << "Partition EOF error: " << message->errstr();
    return nullptr;

  case RdKafka::ERR__UNKNOWN_TOPIC:
    WARN << "Unknown topic: " << message->errstr();
    return nullptr;

  case RdKafka::ERR__UNKNOWN_PARTITION:
    WARN << "Unknown partition: " << message->errstr();
    return nullptr;

  default:
    /* Errors */
    WARN << "Consume failed: " << message->errstr();
    //    WARN << "Failed to consume:" << RdKafka::err2str(msg->err());
    return nullptr;
  }

  return nullptr;
}

Spill* KafkaProducer::messageConsume(std::shared_ptr<RdKafka::Message> msg)
{
  Spill* ret {nullptr};
  if (msg->len() > 0)
  {
    auto em = GetEventMessage(msg->payload());
    std::string name = em->source_name()->str();
    ulong id = em->message_id();
    uint64_t pulse_time = em->pulse_time();
    auto t_len = em->time_of_flight()->Length();
    auto p_len = em->detector_id()->Length();

    if (detector_type_ != name)
    {
      DBG << "Bad detector type " << name << " id=" << id << " pulse=" << pulse_time;
      return ret;
    }

    if ((t_len != p_len) || !t_len)
    {
      DBG << "Empty buffer " << name << " id=" << id << " pulse=" << pulse_time;
      return ret;
    }

    DBG << "Good buffer " << name << " id=" << id << " pulse=" << pulse_time;

    ret = create_spill(StatusType::running);
    int16_t chan0 {0};

    uint64_t time_high = pulse_time << 32;
    for (auto i=0; i < t_len; ++i)
    {
      uint64_t time = em->time_of_flight()->Get(i);
      time |= time_high;

      Event h(chan0, model_hit);
      h.set_native_time(time);
      h.set_value(0, em->detector_id()->Get(i));

      ret->events.push_back(h);

      clock_ = std::max(clock_, time);
    }
  }
  return ret;
}
