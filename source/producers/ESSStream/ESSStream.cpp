#include "ESSStream.h"

#include "custom_logger.h"

#include "ev42_parser.h"
#include "mo01_parser.h"

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


  SettingMeta tm(mp + "TimebaseMult", SettingType::integer, "Timebase multiplier");
  tm.set_val("min", 1);
  tm.set_val("units", "ns");
  add_definition(tm);

  SettingMeta td(mp + "TimebaseDiv", SettingType::integer, "Timebase divider");
  td.set_val("min", 1);
  td.set_val("units", "1/ns");
  add_definition(td);

  SettingMeta spoof(mp + "SpoofClock", SettingType::menu, "Spoof pulse time");
  spoof.set_enum(0, "no");
  spoof.set_enum(1, "monotonous counter");
  spoof.set_enum(2, "use earliest event");
  add_definition(spoof);

  SettingMeta hb(mp + "Heartbeat", SettingType::boolean, "Send empty heartbeat buffers");
  add_definition(hb);

  SettingMeta pars(mp + "Parser", SettingType::menu, "Flatbuffer parser");
  pars.set_enum(0, "none");
  pars.set_enum(1, "ev42_events");
  pars.set_enum(2, "mo01_nmx");
  add_definition(pars);


  SettingMeta root("ESSStream", SettingType::stem);
  root.set_flag("producer");
  root.set_enum(0, mp + "KafkaBroker");
  root.set_enum(1, mp + "KafkaTopic");
  root.set_enum(2, mp + "KafkaTimeout");
  root.set_enum(3, mp + "TimebaseMult");
  root.set_enum(4, mp + "TimebaseDiv");
  root.set_enum(5, mp + "SpoofClock");
  root.set_enum(6, mp + "Heartbeat");
  root.set_enum(7, mp + "Parser");
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

  runner_ = new std::thread(&worker_run, this, out_queue);

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
  set = enrich_and_toggle_presets(set);

  set.set(Setting::text("ESSStream/KafkaBroker", kafka_broker_name_));
  set.set(Setting::text("ESSStream/KafkaTopic", kafka_topic_name_));
  set.set(Setting::integer("ESSStream/KafkaTimeout", kafka_timeout_));

  set.set(Setting::integer("ESSStream/TimebaseMult", time_base_.multiplier()));
  set.set(Setting::integer("ESSStream/TimebaseDiv", time_base_.divider()));

  set.set(Setting::integer("ESSStream/SpoofClock", spoof_clock_));

  while (set.branches.has_a(Setting({"ev42_events", SettingType::stem})))
    set.branches.remove_a(Setting({"ev42_events", SettingType::stem}));

  while (set.branches.has_a(Setting({"mo01_nmx", SettingType::stem})))
    set.branches.remove_a(Setting({"mo01_nmx", SettingType::stem}));

  if (parser_)
  {
    auto s = Setting({parser_->plugin_name(), SettingType::stem});
    parser_->read_settings_bulk(s);
    set.branches.add_a(s);
  }
}


void ESSStream::write_settings_bulk(const Setting& settings)
{
  auto set = enrich_and_toggle_presets(settings);

  kafka_broker_name_ = set.find({"ESSStream/KafkaBroker"}).get_text();
  kafka_topic_name_ = set.find({"ESSStream/KafkaTopic"}).get_text();
  kafka_timeout_ = set.find({"ESSStream/KafkaTimeout"}).get_number();

  time_base_ = TimeBase(set.find({"ESSStream/TimebaseMult"}).get_number(),
                        set.find({"ESSStream/TimebaseDiv"}).get_number());

  spoof_clock_ = set.find({"ESSStream/SpoofClock"}).get_number();

  auto parser_set = set.find({"ESSStream/Parser"});
  auto parser = parser_set.metadata().enum_name(parser_set.selection());

  select_parser(parser);
  if (parser_ && (parser_->plugin_name() == parser))
    parser_->write_settings_bulk(set.find({parser}));
}

void ESSStream::select_parser(std::string t)
{
  if (
      (t.empty() && !parser_) ||
      (parser_ && (t == parser_->plugin_name()))
      )
    return;
  if (t.empty())
    parser_.reset();
  else if (t == "ev42_events")
    parser_ = std::make_shared<ev42_events>();
  else if (t == "mo01_nmx")
    parser_ = std::make_shared<mo01_nmx>();
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

  status_ = ProducerStatus::loaded |
      ProducerStatus::booted | ProducerStatus::can_run;
}

void ESSStream::die()
{
//  INFO << "<ESSStream> Shutting down";
  if (stream_)
  {
    stream_->close();
    // Wait for RdKafka to decommission, avoids complaints of memory leak from
    // valgrind etc.
    RdKafka::wait_destroyed(5000);
    stream_.reset();
  }
  clock_ = 0;
  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

void ESSStream::worker_run(ESSStream* callback, SpillQueue spill_queue)
{
  DBG << "<ESSStream> Starting run   "
      << "  timebase " << callback->time_base_.debug() << "ns";

  if (callback->parser_)
    spill_queue->enqueue(callback->parser_->start_spill());

  uint64_t spills{2};
  callback->time_spent_ = 0;

  while (callback->run_status_.load() != 2)
  {
    auto spill = callback->get_message();
    if (spill)
    {
      spills++;
      spill_queue->enqueue(spill);
    }
  }

  if (callback->parser_)
    spill_queue->enqueue(callback->parser_->stop_spill());

  callback->run_status_.store(3);
  DBG << "<ESSStream> Finished run"
      << " spills=" << spills
      << " time=" << callback->time_spent_
      << " secs/spill=" << callback->time_spent_ / double(spills);
}


SpillPtr ESSStream::get_message()
{
  std::shared_ptr<RdKafka::Message> message
  {stream_->consume(kafka_timeout_)};

  switch (message->err())
  {
  case RdKafka::ERR__UNKNOWN_TOPIC:
    WARN << "Unknown topic: " << message->errstr();
    return nullptr;

  case RdKafka::ERR__UNKNOWN_PARTITION:
    WARN << "Unknown partition: " << message->errstr();
    return nullptr;

  case RdKafka::ERR__TIMED_OUT:
//    return nullptr;

  case RdKafka::ERR__PARTITION_EOF:
    /* Last message */
    //    if (exit_eof && ++eof_cnt == partition_cnt)
    //      WARN << "%% EOF reached for all " << partition_cnt <<
    //                   " partition(s)";
    //    WARN << "Partition EOF error: " << message->errstr();
//    return nullptr;

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

    if (parser_)
    {
      SpillPtr spill;
      parser_->timebase = time_base_;

      if (message->len())
        spill = parser_->process_payload(message->payload(),
                                         (spoof_clock_ == 1) ? (++clock_) : 0);

      if (!spill & heartbeat_ && spoof_clock_)
        spill = parser_->dummy_spill((spoof_clock_ == 1) ? (++clock_) : 0);

      if (spill)
      {
        for (auto& s : spill->stats)
        {
          s.second.set_value("native_time", parser_->stats.time_end);
          if (spoof_clock_ != 0)
            s.second.set_value("pulse_time", parser_->stats.time_start);

//          DBG << "<ESSStream> setting native time (" << s.first << ") "
//              << stats.time_end << " -> " << s.second.stats()["native_time"];
//          DBG << "pulse time (" << s.first << ")"
//                 " = " << s.second.stats()["pulse_time"];
        }
      }

      time_spent_ += parser_->stats.time_spent;
      return spill;
    }


  default:
    WARN << "Consume failed: " << message->errstr();
    return nullptr;
  }

  return nullptr;
}

std::string ESSStream::debug(std::shared_ptr<RdKafka::Message> kmessage)
{
  return std::string(static_cast<const char*>(kmessage->payload()),
                     kmessage->len());
}
