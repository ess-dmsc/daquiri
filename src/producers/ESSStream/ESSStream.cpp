#include "ESSStream.h"
#include "custom_timer.h"

#include "custom_logger.h"

#include "ev42_events_generated.h"
//#include "mon_efu_generated.h"

ESSStream::ESSStream()
{
  std::string mp {"ESSStream/"};


  SettingMeta broker(mp + "KafkaBroker", SettingType::text, "Kafka broker URL");
  broker.set_flag("preset");
  add_definition(broker);

  SettingMeta topic(mp + "KafkaTopic", SettingType::text, "Kafka topic");
  topic.set_flag("preset");
  add_definition(topic);

  SettingMeta pi(mp + "KafkaTimeout", SettingType::integer, "Kafka timeout");
  pi.set_val("min", 1);
  pi.set_val("units", "ms");
  add_definition(pi);

  SettingMeta det_type(mp + "DetectorType", SettingType::text, "Detector type");
  add_definition(det_type);


  SettingMeta tm(mp + "TimebaseMult", SettingType::integer, "Timebase multiplier");
  tm.set_val("min", 1);
  tm.set_val("units", "ns");
  add_definition(tm);

  SettingMeta td(mp + "TimebaseDiv", SettingType::integer, "Timebase divider");
  td.set_val("min", 1);
  td.set_val("units", "1/ns");
  add_definition(td);


  SettingMeta vc(mp + "DimensionCount", SettingType::integer, "Dimensions");
  vc.set_val("min", 1);
  vc.set_val("max", 16);
  add_definition(vc);

  SettingMeta valname(mp + "Dimension/Name", SettingType::text, "Dimension name");
  add_definition(valname);

  SettingMeta pc(mp + "Dimension/Extent", SettingType::integer, "Dimension extent");
  pc.set_val("min", 1);
  add_definition(pc);

  SettingMeta val(mp + "Dimension", SettingType::stem);
  val.set_enum(0, mp + "Dimension/Name");
  val.set_enum(1, mp + "Dimension/Extent");
  add_definition(val);


  SettingMeta root("ESSStream", SettingType::stem);
  root.set_flag("producer");
  root.set_enum(0, mp + "KafkaBroker");
  root.set_enum(1, mp + "KafkaTopic");
  root.set_enum(2, mp + "KafkaTimeout");
  root.set_enum(3, mp + "DetectorType");
  root.set_enum(4, mp + "TimebaseMult");
  root.set_enum(5, mp + "TimebaseDiv");
  root.set_enum(6, mp + "DimensionCount");
  add_definition(root);

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

ESSStream::~ESSStream()
{
  daq_stop();
  if (runner_ != nullptr)
  {
    runner_->detach();
    delete runner_;
  }
  die();
}

bool ESSStream::daq_start(SpillQueue out_queue)
{
  if (run_status_.load() > 0)
    return false;

  run_status_.store(1);

  if (runner_ != nullptr)
    delete runner_;

  runner_ = new boost::thread(&worker_run, this, out_queue);

  return true;
}

bool ESSStream::daq_stop()
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

bool ESSStream::daq_running()
{
  if (run_status_.load() == 3)
    daq_stop();
  return (run_status_.load() > 0);
}

void ESSStream::read_settings_bulk(Setting &set) const
{
  if (set.id() != device_name())
    return;
  set.enrich(setting_definitions_, true);
  set.enable_if_flag(!(status_ & booted), "preset");

  set.set(Setting::text("ESSStream/KafkaBroker", kafka_broker_name_));
  set.set(Setting::text("ESSStream/KafkaTopic", kafka_topic_name_));
  set.set(Setting::integer("ESSStream/KafkaTimeout", kafka_timeout_));

  set.set(Setting::integer("ESSStream/TimebaseMult", model_hit_.timebase.multiplier()));
  set.set(Setting::integer("ESSStream/TimebaseDiv", model_hit_.timebase.divider()));

  set.set(Setting::text("ESSStream/DetectorType", detector_type_));
  set.set(Setting::integer("ESSStream/DimensionCount", integer_t(dim_count_)));

  while (set.branches.has_a(Setting({"ESSStream/Dimension", SettingType::stem})))
    set.branches.remove_a(Setting({"ESSStream/Dimension", SettingType::stem}));

  for (int i=0; i < int(dim_count_); ++i)
  {
    Setting v = get_rich_setting("ESSStream/Dimension");
    v.set_indices({i});
    v.branches = get_rich_setting("ESSStream/Dimension").branches;
    std::string name;
    if (i < geometry_.names_.size())
    {
      name = geometry_.names_.at(i);
      v.set(Setting::text("ESSStream/Dimension/Name", name));
      v.set(Setting::integer("ESSStream/Dimension/Extent",
                             geometry_.dimensions_.at(name)));
    }
    for (auto& vv : v.branches)
      vv.set_indices({i});
    set.branches.add_a(v);
  }
}


void ESSStream::write_settings_bulk(const Setting& settings)
{
  if (settings.id() != device_name())
    return;
  auto set = settings;
  set.enrich(setting_definitions_, true);
  set.enable_if_flag(!(status_ & booted), "preset");

  kafka_broker_name_ = set.find({"ESSStream/KafkaBroker"}).get_text();
  kafka_topic_name_ = set.find({"ESSStream/KafkaTopic"}).get_text();
  kafka_timeout_ = set.find({"ESSStream/KafkaTimeout"}).get_number();

  detector_type_ = set.find({"ESSStream/DetectorType"}).get_text();

  dim_count_ = set.find({"ESSStream/DimensionCount"}).get_number();
  if ((dim_count_ < 1) || (dim_count_ > 16))
    dim_count_ = 1;

  geometry_ = GeometryInterpreter();
  for (Setting v : set.branches)
  {
    if (v.id() != "ESSStream/Dimension")
      continue;
    auto indices = v.indices();
    if (!indices.size())
      continue;
    size_t idx = *indices.begin();
//    DBG << "Write idx " << idx;
    if (idx >= dim_count_)
      continue;
    auto name = v.find({"ESSStream/Dimension/Name"}).get_text();
    size_t ext = v.find({"ESSStream/Dimension/Extent"}).get_number();
    geometry_.add_dimension(name, ext);
  }

  auto timebase = TimeBase(set.find({"ESSStream/TimebaseMult"}).get_number(),
                           set.find({"ESSStream/TimebaseDiv"}).get_number());
  model_hit_ = geometry_.model(timebase);
}

void ESSStream::boot()
{
  if (!(status_ & ProducerStatus::can_boot))
  {
    WARN << "<ESSStream> Cannot boot ESSStream. Failed flag check (can_boot == 0)";
    return;
  }

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;

  INFO << "<ESSStream> Booting";

  std::string error_str;

  auto conf = std::unique_ptr<RdKafka::Conf>(
        RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));

  if (!conf.get())
  {
    ERR << "<ESSStream> Unable to created global Conf object";
    die();
    return;
  }

  conf->set("metadata.broker.list", kafka_broker_name_, error_str);
  conf->set("message.max.bytes", "10000000", error_str);
  conf->set("fetch.message.max.bytes", "10000000", error_str);
  conf->set("replica.fetch.max.bytes", "10000000", error_str);
  conf->set("group.id", "group0", error_str);
  //  conf->set("enable.auto.commit", "false", error_str);
  //  conf->set("enable.auto.offset.store", "false", error_str);
  //  conf->set("offset.store.method", "none", error_str);
  //  conf->set("auto.offset.reset", "largest", error_str);

  stream_ = std::unique_ptr<RdKafka::KafkaConsumer>(
        RdKafka::KafkaConsumer::create(conf.get(), error_str));
  if (!stream_.get())
  {
    ERR << "<ESSStream> Failed to create consumer: " << error_str;
    die();
    return;
  }

  INFO << "<ESSStream> Created consumer " << stream_->name();

  // Start consumer for topic+partition at start offset
  RdKafka::ErrorCode resp = stream_->subscribe({kafka_topic_name_});
  if (resp != RdKafka::ERR_NO_ERROR)
  {
    ERR << "<ESSStream> Failed to start consumer: " << RdKafka::err2str(resp);
    die();
    return;
  }

  clock_ = 0;
  status_ = ProducerStatus::loaded |
      ProducerStatus::booted | ProducerStatus::can_run;
}

void ESSStream::die()
{
  INFO << "<ESSStream> Shutting down";
  if (stream_)
  {
    stream_->close();
    // Wait for RdKafka to decommission, avoids complaints of memory leak from
    // valgrind etc.
    RdKafka::wait_destroyed(5000);
    stream_.reset();
  }
  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

void ESSStream::worker_run(ESSStream* callback,
                               SpillQueue spill_queue)
{
  DBG << "<ESSStream> Starting run   "
      << "  timebase " << callback->model_hit_.timebase.debug() << "ns";

  spill_queue->enqueue(callback->create_spill(StatusType::start));

  while (callback->run_status_.load() != 2)
  {
    auto spill = callback->get_message();
    if (spill)
      spill_queue->enqueue(spill);
  }

  spill_queue->enqueue(callback->create_spill(StatusType::stop));

  callback->run_status_.store(3);
}

Spill* ESSStream::create_spill(StatusType t)
{
  int16_t chan0 {0};
  Spill* spill = new Spill();
  spill->stats[chan0] = get_status(chan0, t);
  return spill;
}

Status ESSStream::get_status(int16_t chan, StatusType t)
{
  Status status;
  status.set_type(t);
  status.set_channel(chan);
  status.set_model(model_hit_);
  status.set_time(boost::posix_time::microsec_clock::universal_time());

  double duration = clock_;

  status.set_value("native_time", duration);
  status.set_value("buf_id", buf_id_);

  return status;
}

Spill* ESSStream::get_message()
{
  std::shared_ptr<RdKafka::Message> message
  {stream_->consume(kafka_timeout_)};

  switch (message->err())
  {
  case RdKafka::ERR__TIMED_OUT:
    return nullptr;

  case RdKafka::ERR_NO_ERROR:
    //    msg_cnt++;
    //    msg_bytes += message->len();
//    DBG << "Read msg at offset " << message->offset();
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

//    DBG << "Received Kafka message " << debug(message);

//    return nullptr;
    return process_message(message);

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

std::string ESSStream::debug(std::shared_ptr<RdKafka::Message> kmessage)
{
  return std::string(static_cast<const char*>(kmessage->payload()),
                     kmessage->len());
}

Spill* ESSStream::process_message(std::shared_ptr<RdKafka::Message> msg)
{
  Spill* ret {nullptr};
  if (msg->len() > 0)
  {
    auto em = GetEventMessage(msg->payload());

    ulong id = em->message_id();
    if (id < buf_id_)
      WARN << "Buffer ID" << id << "out of order "  << debug(*em);
    buf_id_ = std::max(buf_id_, id);

//    if (detector_type_ != em->source_name()->str())
//    {
//      DBG << "Bad detector type " << debug(*em);
//      return ret;
//    }

    auto t_len = em->time_of_flight()->Length();
    auto p_len = em->detector_id()->Length();
    if ((t_len != p_len) || !t_len)
    {
      DBG << "Empty buffer " << debug(*em);
      return ret;
    }

//    DBG << "Good buffer " << debug(*em);

    ret = create_spill(StatusType::running);
    int16_t chan0 {0};

    uint64_t time_high = em->pulse_time();
    time_high = time_high << 32;
    for (auto i=0; i < t_len; ++i)
    {
      uint64_t time = em->time_of_flight()->Get(i);
      time |= time_high;

      Event e(chan0, model_hit_);
      e.set_native_time(time_high);
      geometry_.interpret_id(e, em->detector_id()->Get(i));
      ret->events.push_back(e);

      clock_ = std::max(clock_, time);
    }
  }
  return ret;
}

std::string ESSStream::debug(const EventMessage& em)
{
  std::stringstream ss;

  ss << em.source_name()->str() << " #" << em.message_id()
     << " time=" << em.pulse_time()
     << " tof_size=" << em.time_of_flight()->Length()
     << " det_size=" << em.detector_id()->Length();

  return ss.str();
}
