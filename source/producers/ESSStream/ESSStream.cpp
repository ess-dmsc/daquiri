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

  SettingMeta pars(r + "/Parser", SettingType::menu, "Flatbuffer parser");
  for (auto k : parser_names_)
    pars.set_enum(k.second, k.first);
  add_definition(pars);

  int32_t i {0};
  SettingMeta root(r, SettingType::stem, "ESS Stream");
  root.set_flag("producer");
  root.set_enum(i++, r + "/dummy");
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

  auto parser_set = get_rich_setting(r + "/Parser");
  parser_set.select(0);

  if (parser_)
  {
    parser_set.select(parser_names_.at(parser_->plugin_name()));
    set.branches.add_a(parser_set);
    set.branches.add_a(parser_->settings());
  }
  else
    set.branches.add_a(parser_set);

  set.enable_if_flag(!(status_ & booted), "preset");
  return set;
}

void ESSStream::settings(const Setting& settings)
{
  std::string r {plugin_name()};
  auto set = enrich_and_toggle_presets(settings);

  kafka_config_.settings(set.find({kafka_config_.plugin_name()}));

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
    WARN << "<ESSStream> "
         << "Cannot boot ESSStream. Failed flag check (can_boot == 0)";
    return;
  }

  if (!parser_)
  {
    WARN << "<ESSStream> "
         << "Cannot boot ESSStream. No flat buffer parser specified.";
    return;
  }

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;

  stream_ = kafka_config_.subscribe_topic(parser_->kafka_config.kafka_topic_name_);

  if (!stream_)
  {
    ERR << "<ESSStream:" << parser_->kafka_config.kafka_topic_name_ << "> "
        << "Failed to start consumer.";
    die();
    return;
  }

  INFO << "<ESSStream:" << parser_->kafka_config.kafka_topic_name_ << "> "
       << " booted with consumer " << stream_->low_level->name();

  status_ = ProducerStatus::loaded |
      ProducerStatus::booted | ProducerStatus::can_run;
}

void ESSStream::die()
{
//  INFO << "<ESSStream> Shutting down";
  if (stream_)
  {
    stream_->low_level->close();
    kafka_config_.decomission();
    stream_.reset();
  }
  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

void ESSStream::worker_run(SpillQueue spill_queue)
{
  DBG << "<ESSStream:" << parser_->kafka_config.kafka_topic_name_ << "> "
      << "Starting run"; //more info!!!

  uint64_t spills {0};

  while (!terminate_.load())
    spills += get_message(spill_queue);

  if (parser_)
    spills += parser_->stop(spill_queue);

  DBG << "<ESSStream:" << parser_->kafka_config.kafka_topic_name_ << "> "
      << "Finished run"
      << "  spills=" << spills;

  if (parser_)
    DBG << "<ESSStream:" << parser_->kafka_config.kafka_topic_name_ << "> "
        << "  time=" << parser_->stats.time_spent
        << "  secs/spill=" << parser_->stats.time_spent / double(spills)
        << "  skipped buffers=" << parser_->stats.dropped_buffers;
}

bool ESSStream::good(Kafka::MessagePtr message)
{
  switch (message->low_level->err())
  {
    case RdKafka::ERR__UNKNOWN_TOPIC:
      WARN << "<ESSStream> Unknown topic! Err=" << message->low_level->errstr();
      return false;

    case RdKafka::ERR__UNKNOWN_PARTITION:
      WARN << "<ESSStream> topic:" << message->low_level->topic_name()
           << "  Unknown partition! Err=" << message->low_level->errstr();
      return false;

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
      ts = message->low_level->timestamp();
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

      return (message->low_level->len() > 0);

    default:
      WARN << "<ESSStream> " << message->low_level->topic_name()
           << ":" << message->low_level->partition()
           << " Consume failed! Err=" << message->low_level->errstr();
      return false;
  }
}

uint64_t ESSStream::get_message(SpillQueue spill_queue)
{
  auto message = stream_->consume(kafka_config_.kafka_timeout_);

  if (!good(message))
    return 0;

  uint64_t num_produced {0};

  if (parser_)
  {
    num_produced = parser_->process_payload(spill_queue, message->low_level->payload());
    if (parser_->kafka_config.kafka_ff_)
      parser_->stats.dropped_buffers += ff_stream(stream_, message, parser_->kafka_config.kafka_max_backlog_);
  }
  else
    WARN << "<ESSStream:" << parser_->kafka_config.kafka_topic_name_ << "> "
         << "Consume failed. No parser to interpret buffer.";

  return num_produced;
}

uint64_t ESSStream::ff_stream(Kafka::ConsumerPtr consumer, Kafka::MessagePtr message,
                              int64_t kafka_max_backlog)
{
  auto offsets = consumer->get_watermark_offsets(message);

  if ((offsets.hi - message->low_level->offset()) > kafka_max_backlog)
  {
    DBG << "<ESSStream> topic:" << message->low_level->topic_name() << " "
        << " partition: " << message->low_level->partition()
        << " Backlog exceeded with offset=" << message->low_level->offset()
        << "  hi=" << offsets.hi;

    consumer->seek(message, offsets.hi, 2000);
    return (offsets.hi - message->low_level->offset());
  }
  return 0;
}
