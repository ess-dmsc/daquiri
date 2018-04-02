#include "ESSStream.h"
#include "ev42_parser.h"
#include "mo01_parser.h"
#include "f142_parser.h"

#include "custom_logger.h"

ESSStream::ESSStream()
{
  parser_names_["none"] = 0;
  parser_names_["ev42_events"] = 1;
  parser_names_["mo01_nmx"] = 2;
  parser_names_["ChopperTDC"] = 3;

  std::string r {plugin_name()};

  SettingMeta topic(r + "/KafkaTopic", SettingType::text, "Kafka topic");
  topic.set_flag("preset");
  add_definition(topic);

  SettingMeta drop {r + "/KafkaFF", SettingType::boolean, "Fast-forward to recent packets"};
  add_definition(drop);

  SettingMeta mb(r + "/KafkaMaxBacklog", SettingType::integer, "Kafka maximum backlog");
  mb.set_val("min", 1);
  mb.set_val("units", "buffers");
  add_definition(mb);

  SettingMeta pars(r + "/Parser", SettingType::menu, "Flatbuffer parser");
  for (auto k : parser_names_)
    pars.set_enum(k.second, k.first);
  add_definition(pars);

  int32_t i {0};
  SettingMeta root(r, SettingType::stem);
  root.set_flag("producer");
  root.set_enum(i++, r + "/KafkaTopic");
  root.set_enum(i++, r + "/KafkaFF");
  root.set_enum(i++, r + "/KafkaMaxBacklog");
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

StreamManifest ESSStream::stream_manifest() const
{
  if (parser_)
    return parser_->stream_manifest();
  return StreamManifest();
}

Setting ESSStream::settings() const
{
  std::string r {plugin_name()};
  auto set = get_rich_setting(r);

  set.branches.add_a(kafka_config_.settings());

  set.set(Setting::text(r + "/KafkaTopic", kafka_topic_name_));
  set.set(Setting::boolean(r + "/KafkaFF", kafka_ff_));
  set.set(Setting::integer(r + "/KafkaMaxBacklog", kafka_max_backlog_));

  auto parser_set = get_rich_setting(r + "/Parser");

  if (parser_)
  {
    parser_set.select(parser_names_.at(parser_->plugin_name()));
    set.branches.add_a(parser_set);
    set.branches.add_a(parser_->settings());
  }

  set.enable_if_flag(!(status_ & booted), "preset");
  return set;
}

void ESSStream::settings(const Setting& settings)
{
  std::string r {plugin_name()};

  auto set = enrich_and_toggle_presets(settings);

  kafka_config_.settings(set.find({kafka_config_.plugin_name()}));

  kafka_topic_name_ = set.find({r + "/KafkaTopic"}).get_text();
  kafka_ff_ = set.find({r + "/KafkaFF"}).get_bool();
  kafka_max_backlog_ = set.find({r + "/KafkaMaxBacklog"}).get_int();

  auto parser_set = set.find({r + "/Parser"});
  auto parser = parser_set.metadata().enum_name(parser_set.selection());

  select_parser(parser);
  if (parser_ && (parser_->plugin_name() == parser))
    parser_->settings(set.find({parser}));
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
  else if (t == "ChopperTDC")
    parser_ = std::make_shared<ChopperTDC>();
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

  stream_ = kafka_config_.subscribe_topic(kafka_topic_name_);

  if (!stream_)
  {
    ERR << "<ESSStream:" << kafka_topic_name_ << "> "
        << "Failed to start consumer.";
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
  if (stream_)
  {
    stream_->close();
    kafka_config_.decomission();
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
      << "  skipped buffers=" << dropped_buffers_;
}

uint64_t ESSStream::get_message(SpillQueue spill_queue)
{
  std::shared_ptr<RdKafka::Message> message
      {stream_->consume(kafka_config_.kafka_timeout_)};

  switch (message->err())
  {
    case RdKafka::ERR__UNKNOWN_TOPIC:WARN << "<ESSStream:" << kafka_topic_name_ << "> "
                                          << "Unknown topic: " << message->errstr();
      return 0;

    case RdKafka::ERR__UNKNOWN_PARTITION:WARN << "<ESSStream:" << kafka_topic_name_ << "> "
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

        if (kafka_ff_)
          ff_stream(message);

        uint64_t num_produced = parser_->process_payload(spill_queue, message->payload());

        time_spent_ += parser_->stats.time_spent;
        return num_produced;
      } else
      {
        WARN << "<ESSStream:" << kafka_topic_name_ << "> "
             << "Consume failed. No parser to interpret buffer.";
        return 0;
      }

    default:WARN << "<ESSStream:" << kafka_topic_name_ << "> "
                 << "Consume failed: " << message->errstr();
      return 0;
  }

  return 0;
}

void ESSStream::ff_stream(std::shared_ptr<RdKafka::Message> message)
{
  int64_t hi_o {0}, lo_o {0};
  stream_->get_watermark_offsets(kafka_topic_name_,
                                 message->partition(),
                                 &lo_o, &hi_o);

  if ((hi_o - message->offset()) > kafka_max_backlog_)
  {
    DBG << "<ESSStream:" << kafka_topic_name_ << "> "
        << "Backlog exceeded on partition " << message->partition()
        << " offset=" << message->offset() << "  hi=" << hi_o;

    kafka_config_.seek(stream_, kafka_topic_name_, message->partition(), hi_o);
    dropped_buffers_ += (hi_o - message->offset());
  }
}
