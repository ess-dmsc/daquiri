#include <producers/ESSStream/ESSStream.h>

#include <producers/ESSStream/ev42_parser.h>
#include <producers/ESSStream/mo01_parser.h>
#include <producers/ESSStream/f142_parser.h>
#include <producers/ESSStream/senv_data_parser.h>
#include <producers/ESSStream/senv_data_wrong.h>

#include <core/util/logger.h>

ESSStream::ESSStream()
{
  parser_names_["none"] = 0;
  parser_names_["ev42_events"] = 1;
  parser_names_["mo01_nmx"] = 2;
  parser_names_["ChopperTDC"] = 3;
  parser_names_["SenvParser"] = 4;
  parser_names_["SenvParserWrong"] = 5;

  std::string r {plugin_name()};

  SettingMeta pars(r + "/Parser", SettingType::menu, "Flatbuffer parser");
  for (auto k : parser_names_)
    pars.set_enum(k.second, k.first);
  add_definition(pars);

  SettingMeta vc(r + "/TopicCount", SettingType::integer, "Topic count");
  vc.set_flag("preset");
  vc.set_val("min", 1);
  vc.set_val("max", 16);
  add_definition(vc);

  SettingMeta st(r + "/Topic", SettingType::stem, "Kafka topic");
  add_definition(st);

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

  running_.store(true);
  terminate_.store(false);

  size_t total = 0;
  for (auto& s : streams_)
  {
    if (!s.parser)
    {
      WARN("<ESSStream> Could not start worker cuz bad parser");
      continue;
    }

    if (!s.consumer)
    {
      WARN("<ESSStream> Could not start worker cuz bad kafka stream");
      continue;
    }

    s.runner = std::thread(&ESSStream::Stream::worker_run, &s, out_queue,
                           kafka_config_.kafka_timeout_,
                           &terminate_);
    total++;
  }

  return total;
}

bool ESSStream::daq_stop()
{
  terminate_.store(true);

  for (auto& s : streams_)
    if (s.runner.joinable())
      s.runner.join();

  running_.store(false);

  return true;
}

bool ESSStream::daq_running()
{
  return (running_.load());
}

StreamManifest ESSStream::stream_manifest() const
{
  StreamManifest ret;
  for (auto& s : streams_)
  {
    if (!s.parser)
      continue;
    for (auto m : s.parser->stream_manifest())
      ret[m.first] = m.second;
  }
  return ret;
}

Setting ESSStream::settings() const
{
  std::string r {plugin_name()};
  auto set = get_rich_setting(r);

  set.branches.add_a(kafka_config_.settings());

  auto tc = get_rich_setting(r + "/TopicCount");
  tc.set_int(streams_.size());
  set.branches.add_a(tc);

  for (auto& s : streams_)
  {
    auto topic = get_rich_setting(r + "/Topic");

    topic.branches.add_a(s.config.settings());

    auto parser_set = get_rich_setting(r + "/Parser");
    parser_set.select(0);
    if (s.parser)
    {
      parser_set.select(parser_names_.at(s.parser->plugin_name()));
      topic.branches.add_a(parser_set);
      topic.branches.add_a(s.parser->settings());
    } else
      topic.branches.add_a(parser_set);

    set.branches.add_a(topic);
  }

  set.enable_if_flag(!(status_ & booted), "preset");
  return set;
}

void ESSStream::settings(const Setting& settings)
{
  std::string r {plugin_name()};
  auto set = enrich_and_toggle_presets(settings);

  kafka_config_.settings(set.find({kafka_config_.plugin_name()}));

  size_t total = set.find({r + "/TopicCount"}).get_int();
  if (!total)
    total = 1;

  streams_.resize(total);

  size_t i = 0;
  for (Setting v : set.branches)
  {
    if (i >= streams_.size())
      break;
    if (v.id() != (r + "/Topic"))
      continue;

    auto parser_set = enrich_and_toggle_presets(v.find({r + "/Parser"}));
    auto parser = parser_set.metadata().enum_name(parser_set.selection());

    streams_[i].config.settings(v.find(streams_[i].config.plugin_name()));

    select_parser(i, parser);
    if (streams_[i].parser && (streams_[i].parser->plugin_name() == parser))
      streams_[i].parser->settings(v.find({parser}));
    i++;
  }
}

void ESSStream::select_parser(size_t i, std::string t)
{
  if (
      (t.empty() && !streams_[i].parser) ||
          (streams_[i].parser && (t == streams_[i].parser->plugin_name()))
      )
    return;
  if (t.empty())
    streams_[i].parser.reset();
  else if (t == "ev42_events")
    streams_[i].parser = std::make_shared<ev42_events>();
  else if (t == "mo01_nmx")
    streams_[i].parser = std::make_shared<mo01_nmx>();
  else if (t == "ChopperTDC")
    streams_[i].parser = std::make_shared<ChopperTDC>();
  else if (t == "SenvParser")
    streams_[i].parser = std::make_shared<SenvParser>();
  else if (t == "SenvParserWrong")
    streams_[i].parser = std::make_shared<SenvParserWrong>();
}

void ESSStream::boot()
{
  if (!(status_ & ProducerStatus::can_boot))
  {
    WARN("<ESSStream> Cannot boot ESSStream. Failed flag check (can_boot == 0)");
    return;
  }

  size_t total_valid = 0;
  for (auto& s : streams_)
  {
    if (!s.parser)
    {
      WARN("<ESSStream> Cannot boot ESSStream. No flat buffer parser specified.");
      continue;
    }

    status_ = ProducerStatus::loaded | ProducerStatus::can_boot;

    s.consumer = kafka_config_.subscribe_topic(s.config.kafka_topic_name_);

    if (!s.consumer)
    {
      ERR("<ESSStream:{}> Failed to start consumer.", s.config.kafka_topic_name_);
      continue;
    }

    INFO("<ESSStream:{}> booted with consumer {}",
        s.config.kafka_topic_name_, s.consumer->low_level->name());
    total_valid++;
  }

  if (!total_valid)
  {
    ERR("<ESSStream> Failed to start at least one consumers");
    die();
    return;
  }

  status_ = ProducerStatus::loaded |
      ProducerStatus::booted | ProducerStatus::can_run;
}

void ESSStream::die()
{
//  INFO( "<ESSStream> Shutting down";
  for (auto& s : streams_)
    if (s.consumer)
      s.consumer->low_level->close();

  kafka_config_.decomission();

  for (auto& s : streams_)
    if (s.consumer)
      s.consumer.reset();

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

void ESSStream::Stream::worker_run(SpillQueue spill_queue,
                                   uint16_t consume_timeout,
                                   std::atomic<bool>* terminate)
{
  DBG("<ESSStream:{}> Starting run", config.kafka_topic_name_); //more info!!!

  uint64_t spills {0};

  while (!terminate->load())
  {
    auto message = consumer->consume(consume_timeout);

    if (!good(message))
      continue;

    if (get_fb_id(message) != parser->schema_id())
    {
//      auto ch = reinterpret_cast<char const *const>(message->low_level->payload());
//      std::string c(ch, message->low_level->len());
//      WARN("<ESSStream:{}> fb schema id mismatch {}!={}\ncontents:{}",
//          config.kafka_topic_name_, get_fb_id(message), parser->schema_id());
      continue;
    }

    spills += parser->process_payload(spill_queue, message->low_level->payload());

    if (config.kafka_ff_)
        parser->stats.dropped_buffers +=
            ff_stream(message, config.kafka_max_backlog_);
  }

  spills += parser->stop(spill_queue);

  DBG("<ESSStream:{}> Finished run, spills={}", config.kafka_topic_name_, spills);

  DBG("<ESSStream:{}>   time={}  secs/spill={}  skipped buffers={}",
      config.kafka_topic_name_, parser->stats.time_spent,
      parser->stats.time_spent / double(spills),
      parser->stats.dropped_buffers);
}

bool ESSStream::good(Kafka::MessagePtr message)
{
  switch (message->low_level->err())
  {
    case RdKafka::ERR__UNKNOWN_TOPIC:
      WARN("<ESSStream> Unknown topic! Err={}", message->low_level->errstr());
      return false;

    case RdKafka::ERR__UNKNOWN_PARTITION:
      WARN("<ESSStream> topic:{}  Unknown partition! Err={}",
          message->low_level->topic_name(), message->low_level->errstr());
      return false;

    case RdKafka::ERR__TIMED_OUT:
      //WARN( "Kafka time out error: {}", message->low_level->errstr());
      return false;

    case RdKafka::ERR__PARTITION_EOF:
      //WARN( "Kafka partition EOF error: {}", message->low_level->errstr());
      return false;

          /* Last message */
      //    if (exit_eof && ++eof_cnt == partition_cnt)
      //      WARN( "%% EOF reached for all " << partition_cnt <<
      //                   " partition(s)";
      //    WARN( "Partition EOF error: " << message->errstr();
//    return 0;

    case RdKafka::ERR_NO_ERROR:
      //    msg_cnt++;
      //    msg_bytes += message->len();
      //    DBG( "Read msg at offset " << message->offset();
      RdKafka::MessageTimestamp ts;
      ts = message->low_level->timestamp();
      if (ts.type != RdKafka::MessageTimestamp::MSG_TIMESTAMP_NOT_AVAILABLE)
      {
        std::string tsname = "?";
        if (ts.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_CREATE_TIME)
          tsname = "create time";
        else if (ts.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_LOG_APPEND_TIME)
          tsname = "log append time";
//      DBG( "Timestamp: " << tsname << " " << ts.timestamp;
      }


      //    DBG( "Received Kafka message " << debug(message);

      return (message->low_level->len() > 0);

    default:
      WARN("<ESSStream> {}:{} Consume failed! Err={}",
          message->low_level->topic_name(),
          message->low_level->partition(),
          message->low_level->errstr());
      return false;
  }
}

std::string ESSStream::get_fb_id(Kafka::MessagePtr message)
{
  if (message->low_level->len() < 8)
  {
    ERR("Could not extract id. Flatbuffer was only {} bytes. Expected ≥ 8 bytes.", message->low_level->len());
    return {};
  }
  auto ch = reinterpret_cast<char const *const>(message->low_level->payload());
  return std::string(ch + 4, 4);
}


uint64_t ESSStream::Stream::ff_stream(Kafka::MessagePtr message,
                                      int64_t kafka_max_backlog)
{
  auto offsets = consumer->get_watermark_offsets(message);

  if ((offsets.hi - message->low_level->offset()) > kafka_max_backlog)
  {
    DBG("<ESSStream> topic:{} partition:{} Backlog exceeded with offset={}  hi={}",
        message->low_level->topic_name(), message->low_level->partition(),
        message->low_level->offset(), offsets.hi);

    consumer->seek(message, offsets.hi, 2000);
    return (offsets.hi - message->low_level->offset());
  }
  return 0;
}
