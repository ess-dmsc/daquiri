#include "ESSStream.h"
#include "ev42_parser.h"
#include "mo01_parser.h"

#include "custom_logger.h"

ESSStream::ESSStream()
{
  std::string mp {"ESSStream/"};

  SettingMeta broker(mp + "KafkaBroker", SettingType::text, "Kafka broker URL");
  broker.set_flag("preset");
  add_definition(broker);

  SettingMeta topic(mp + "KafkaTopic", SettingType::text, "Kafka topic");
  topic.set_flag("preset");
  add_definition(topic);

  SettingMeta pi(mp + "KafkaTimeout", SettingType::integer, "Kafka consume timeout");
  pi.set_val("min", 1);
  pi.set_val("units", "ms");
  add_definition(pi);

  SettingMeta ti(mp + "KafkaDecomission", SettingType::integer, "Kafka termination timeout");
  ti.set_val("min", 1);
  ti.set_val("units", "ms");
  add_definition(ti);

  SettingMeta mb(mp + "KafkaMaxBacklog", SettingType::integer, "Kafka maximum backlog");
  mb.set_val("min", 1);
  mb.set_val("units", "buffers");
  add_definition(mb);

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
  root.set_enum(3, mp + "KafkaDecomission");
  root.set_enum(4, mp + "KafkaMaxBacklog");
  root.set_enum(7, mp + "Parser");
  add_definition(root);

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

ESSStream::~ESSStream()
{
  daq_stop();
  die();
}

bool ESSStream::daq_start(SpillQueue out_queue)
{
  if (running_.load())
    daq_stop();

  terminate_.store(false);
  running_.store(true);
  runner_ = std::thread(&ESSStream::worker_run, this, out_queue);

  return true;
}

bool ESSStream::daq_stop()
{
  terminate_.store(true);

  if (runner_.joinable())
    runner_.join();
  running_.store(false);

  return true;
}

bool ESSStream::daq_running()
{
  return (running_.load());
}

void ESSStream::read_settings_bulk(Setting &set) const
{
  set = enrich_and_toggle_presets(set);

  set.set(Setting::text("ESSStream/KafkaBroker", kafka_broker_name_));
  set.set(Setting::text("ESSStream/KafkaTopic", kafka_topic_name_));
  set.set(Setting::integer("ESSStream/KafkaTimeout", kafka_timeout_));
  set.set(Setting::integer("ESSStream/KafkaDecomission", kafka_decomission_wait_));
  set.set(Setting::integer("ESSStream/KafkaMaxBacklog", kafka_max_backlog_));

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
  kafka_decomission_wait_ = set.find({"ESSStream/KafkaDecomission"}).get_number();
  kafka_max_backlog_ = set.find({"ESSStream/KafkaMaxBacklog"}).get_number();

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
    WARN << "<ESSStream:" << kafka_topic_name_ << "> "
         << "Cannot boot ESSStream. Failed flag check (can_boot == 0)";
    return;
  }

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;

  auto conf = std::unique_ptr<RdKafka::Conf>(
        RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));

  if (!conf.get())
  {
    ERR << "<ESSStream:" << kafka_topic_name_ << "> "
        << "Unable to created global Conf object";
    die();
    return;
  }

  std::string error_str;

  conf->set("metadata.broker.list", kafka_broker_name_, error_str);
  conf->set("message.max.bytes", "10000000", error_str);
  conf->set("fetch.message.max.bytes", "10000000", error_str);
//  conf->set("replica.fetch.max.bytes", "10000000", error_str);
  conf->set("group.id", "group0", error_str);
  //  conf->set("enable.auto.commit", "false", error_str);
  //  conf->set("enable.auto.offset.store", "false", error_str);
  //  conf->set("offset.store.method", "none", error_str);
  //  conf->set("auto.offset.reset", "largest", error_str);

  stream_ = std::unique_ptr<RdKafka::KafkaConsumer>(
        RdKafka::KafkaConsumer::create(conf.get(), error_str));
  if (!stream_.get())
  {
    ERR << "<ESSStream:" << kafka_topic_name_ << "> "
        << "Failed to create consumer: " << error_str;
    die();
    return;
  }

  // Start consumer for topic+partition at start offset
  RdKafka::ErrorCode resp = stream_->subscribe({kafka_topic_name_});
  if (resp != RdKafka::ERR_NO_ERROR)
  {
    ERR << "<ESSStream:" << kafka_topic_name_ << "> "
        << "Failed to start consumer: " << RdKafka::err2str(resp);
    die();
    return;
  }

  try
  {
    partitions_ = get_partitions();
  }
  catch (...)
  {
    ERR << "<ESSStream:" << kafka_topic_name_ << "> "
        << "Failed to retrieve topic partitions";
    die();
    return;
  }

  if (partitions_.empty())
  {
    ERR << "<ESSStream:" << kafka_topic_name_ << "> "
        << "No partitions found for given topic";
    die();
    return;
  }

  INFO << "<ESSStream:" << kafka_topic_name_ << "> "
       << " booted with consumer " << stream_->name();

  status_ = ProducerStatus::loaded |
      ProducerStatus::booted | ProducerStatus::can_run;
}

void ESSStream::die()
{
//  INFO << "<ESSStream> Shutting down";
  if (partitions_.size())
    RdKafka::TopicPartition::destroy(partitions_);
  if (stream_)
  {
    stream_->close();
    // Wait for RdKafka to decommission, avoids complaints of memory leak from
    // valgrind etc.
    RdKafka::wait_destroyed(kafka_decomission_wait_);
    stream_.reset();
  }
  clock_ = 0;
  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

void ESSStream::worker_run(SpillQueue spill_queue)
{
  DBG << "<ESSStream:" << kafka_topic_name_ << "> "
      << "Starting run"; //more info!!!

  uint64_t spills {0};

  time_spent_ = 0;
  dropped_buffers_ = 0;

  while (!terminate_.load())
    spills += get_message(spill_queue);

  if (parser_)
    spills += parser_->stop(spill_queue);

  DBG << "<ESSStream:" << kafka_topic_name_ << "> "
      << "Finished run"
      << "  spills=" << spills
      << "  time=" << time_spent_
      << "  secs/spill=" << time_spent_ / double(spills)
      << "  skipped buffers=" << dropped_buffers_
         ;
}


uint64_t ESSStream::get_message(SpillQueue spill_queue)
{
  std::shared_ptr<RdKafka::Message> message
  {stream_->consume(kafka_timeout_)};

  switch (message->err())
  {
  case RdKafka::ERR__UNKNOWN_TOPIC:
    WARN << "<ESSStream:" << kafka_topic_name_ << "> "
         << "Unknown topic: " << message->errstr();
    return 0;

  case RdKafka::ERR__UNKNOWN_PARTITION:
    WARN << "<ESSStream:" << kafka_topic_name_ << "> "
         << "Unknown partition: " << message->errstr();
    return 0;

  case RdKafka::ERR__TIMED_OUT:
//    return 0;

  case RdKafka::ERR__PARTITION_EOF:
    /* Last message */
    //    if (exit_eof && ++eof_cnt == partition_cnt)
    //      WARN << "%% EOF reached for all " << partition_cnt <<
    //                   " partition(s)";
    //    WARN << "Partition EOF error: " << message->errstr();
//    return 0;

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
//      DBG << "Timestamp: " << tsname << " " << ts.timestamp;
    }


    //    DBG << "Received Kafka message " << debug(message);

    if (parser_)
    {
      if (!message->len())
        return 0;

      int64_t hi_o {0}, lo_o {0};
      stream_->get_watermark_offsets(kafka_topic_name_,
                                     message->partition(),
                                     &lo_o, &hi_o);

      if ((hi_o - message->offset()) > kafka_max_backlog_)
      {
        DBG << "<ESSStream:" << kafka_topic_name_ << "> "
            << "Backlog exceeded on partition " << message->partition()
            << " offset=" << message->offset() << "  hi=" << hi_o;

        dropped_buffers_ += (hi_o - message->offset());

        auto topicPartition = partitions_[message->partition()];
        topicPartition->set_offset(hi_o);
//        stream_->offsetsForTimes(partitions_, 10000);
//        stream_->seek(*topicPartition, 0);
        stream_->assign(partitions_);
      }

      uint64_t num_produced = parser_->process_payload(spill_queue, message->payload());

      time_spent_ += parser_->stats.time_spent;
      return num_produced;
    }
    else
    {
      WARN << "<ESSStream:" << kafka_topic_name_ << "> "
           << "Consume failed. No parser to inerpret buffer.";
      return 0;
    }

  default:
    WARN << "<ESSStream:" << kafka_topic_name_ << "> "
         << "Consume failed: " << message->errstr();
    return 0;
  }

  return 0;
}

std::vector<RdKafka::TopicPartition*> ESSStream::get_partitions()
{
  std::vector<RdKafka::TopicPartition*> partitions;
  auto metadata = get_kafka_metadata();
  auto topics = metadata->topics();
  if (topics->empty())
    return partitions;

  const RdKafka::TopicMetadata* tmet {nullptr};
  for (auto t : *topics)
  {
    if (t->topic() == kafka_topic_name_)
      tmet = t;
  }

  if (!tmet)
    return partitions;

  auto partitionMetadata = tmet->partitions();
  // Create a TopicPartition for each partition in the topic
  for (size_t partitionNumber = 0;
       partitionNumber < partitionMetadata->size();
       ++partitionNumber)
  {
    auto topicPartition =
        RdKafka::TopicPartition::create(kafka_topic_name_, static_cast<int>(partitionNumber));
    partitions.push_back(topicPartition);
  }
  return partitions;
}

std::unique_ptr<RdKafka::Metadata> ESSStream::get_kafka_metadata() const
{
  RdKafka::Metadata* metadataRawPtr(nullptr);
  // API requires address of a pointer to the struct but compiler won't allow
  // &metadata.get() as it is an rvalue
  stream_->metadata(true, nullptr, &metadataRawPtr, kafka_timeout_);
  // Capture the pointer in an owning struct to take care of deletion
  std::unique_ptr<RdKafka::Metadata> metadata(std::move(metadataRawPtr));
  if (!metadata)
  {
    throw std::runtime_error("Failed to query metadata from broker");
  }
  return metadata;
}

std::string ESSStream::debug(std::shared_ptr<RdKafka::Message> kmessage)
{
  return std::string(static_cast<const char*>(kmessage->payload()),
                     kmessage->len());
}
