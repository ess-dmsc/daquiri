#include <producers/ESSStream/KafkaPlugin.h>
#include <core/util/logger.h>

namespace Kafka {

std::string Message::print_data() const
{
  return std::string(static_cast<const char*>(low_level->payload()),
                     low_level->len());
}

MessagePtr Consumer::consume(uint16_t timeout_ms)
{
  return std::make_shared<Message>(low_level->consume(timeout_ms));
}

Offsets Consumer::get_watermark_offsets(const std::string& topic, int32_t partition)
{
  Offsets ret;
  low_level->get_watermark_offsets(topic, partition, &ret.lo, &ret.hi);
  return ret;
}

Offsets Consumer::get_watermark_offsets(MessagePtr msg)
{
  return get_watermark_offsets(msg->low_level->topic_name(), msg->low_level->partition());
}

void Consumer::seek(const std::string& topic, int32_t partition,
          int64_t offset, int timeout_ms)
{
  auto topicPartition = RdKafka::TopicPartition::create(topic, partition);
  topicPartition->set_offset(offset);
  auto error = low_level->seek(*topicPartition, timeout_ms);
  if (error)
  {
    std::ostringstream os;
    os << "Offset seek failed with error: '" << err2str(error) << "'";
    throw std::runtime_error(os.str());
  }
}

void Consumer::seek(MessagePtr msg, int64_t offset, int timeout_ms)
{
  seek(msg->low_level->topic_name(), msg->low_level->partition(),
       offset, timeout_ms);
}


}


using namespace DAQuiri;

KafkaConfigPlugin::KafkaConfigPlugin()
{
  std::string r {plugin_name()};

  SettingMeta broker(r + "/KafkaBroker", SettingType::text, "Kafka broker URL");
  broker.set_flag("preset");
  add_definition(broker);

  SettingMeta pi(r + "/KafkaTimeout", SettingType::integer, "Kafka consume timeout");
  pi.set_val("min", 1);
  pi.set_val("units", "ms");
  add_definition(pi);

  SettingMeta ti(r + "/KafkaDecomission", SettingType::integer, "Kafka termination timeout");
  ti.set_val("min", 1);
  ti.set_val("units", "ms");
  add_definition(ti);

  int32_t i {0};
  SettingMeta root(r, SettingType::stem, "Kafka broker configuration");
  root.set_enum(i++, r + "/KafkaBroker");
  root.set_enum(i++, r + "/KafkaTimeout");
  root.set_enum(i++, r + "/KafkaDecomission");
  add_definition(root);
}

Setting KafkaConfigPlugin::settings() const
{
  std::string r {plugin_name()};
  auto set = get_rich_setting(r);
  set.set(Setting::text(r + "/KafkaBroker", kafka_broker_name_));
  set.set(Setting::integer(r + "/KafkaTimeout", kafka_timeout_));
  set.set(Setting::integer(r + "/KafkaDecomission", kafka_decomission_wait_));
  return set;
}

void KafkaConfigPlugin::settings(const Setting& set)
{
  std::string r {plugin_name()};
  kafka_broker_name_ = set.find({r + "/KafkaBroker"}).get_text();
  kafka_timeout_ = set.find({r + "/KafkaTimeout"}).get_int();
  kafka_decomission_wait_ = set.find({r + "/KafkaDecomission"}).get_int();
}

std::string KafkaConfigPlugin::random_string( size_t length )
{
  auto randchar = []() -> char
  {
    const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[ rand() % max_index ];
  };
  std::string str(length,0);
  std::generate_n( str.begin(), length, randchar );
  return str;
}

Kafka::ConsumerPtr KafkaConfigPlugin::subscribe_topic(std::string topic) const
{
  auto conf = std::unique_ptr<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));

  if (!conf.get())
  {
    ERR("<KafkaConfigPlugin> Unable to created global Conf object");
    return nullptr;
  }

  std::string error_str;

  conf->set("metadata.broker.list", kafka_broker_name_, error_str);
  conf->set("message.max.bytes", "10000000", error_str);
  conf->set("fetch.message.max.bytes", "10000000", error_str);
  conf->set("replica.fetch.max.bytes", "10000000", error_str);
  conf->set("group.id", random_string(16), error_str);
  conf->set("enable.auto.commit", "false", error_str);
  conf->set("enable.auto.offset.store", "false", error_str);
  conf->set("offset.store.method", "none", error_str);
  //  conf->set("auto.offset.reset", "largest", error_str);
  //  conf->set("session.timeout.ms", "10000", error_str);
  //  conf->set("api.version.request", "true", error_str);

  auto ret = std::make_shared<Kafka::Consumer>
      (RdKafka::KafkaConsumer::create(conf.get(), error_str));
  if (!ret->low_level.get())
  {
    ERR("<KafkaConfigPlugin> Failed to create consumer:{}", error_str);
    return nullptr;
  }

  // Start consumer for topic+partition at start offset
  RdKafka::ErrorCode resp = ret->low_level->subscribe({topic});
  if (resp != RdKafka::ERR_NO_ERROR)
  {
    ERR("<KafkaConfigPlugin> Failed to subscribe consumer to '{}': {}",
        topic, err2str(resp));
    ret->low_level->close();
    decomission();
    ret.reset();
  }

  return ret;
}

void KafkaConfigPlugin::decomission() const
{
  // Wait for RdKafka to decommission, avoids complaints of memory leak from
  // valgrind etc.
  RdKafka::wait_destroyed(kafka_decomission_wait_);

  // deal with return value?
}

std::vector<RdKafka::TopicPartition*> KafkaConfigPlugin::get_partitions(std::shared_ptr<RdKafka::KafkaConsumer> consumer, std::string topic)
{
  std::vector<RdKafka::TopicPartition*> partitions;
  auto metadata = get_kafka_metadata(consumer);
  auto topics = metadata->topics();
  if (topics->empty())
    return partitions;

  const RdKafka::TopicMetadata* tmet {nullptr};
  for (auto t : *topics)
  {
    if (t->topic() == topic)
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
        RdKafka::TopicPartition::create(topic, static_cast<int>(partitionNumber));
    partitions.push_back(topicPartition);
  }
  return partitions;
}

std::unique_ptr<RdKafka::Metadata> KafkaConfigPlugin::get_kafka_metadata(std::shared_ptr<RdKafka::KafkaConsumer> consumer) const
{
  RdKafka::Metadata* metadataRawPtr(nullptr);
  // API requires address of a pointer to the struct but compiler won't allow
  // &metadata.get() as it is an rvalue
  consumer->metadata(true, nullptr, &metadataRawPtr, kafka_timeout_);
  // Capture the pointer in an owning struct to take care of deletion
  std::unique_ptr<RdKafka::Metadata> metadata(std::move(metadataRawPtr));
  if (!metadata)
  {
    throw std::runtime_error("Failed to query metadata from broker");
  }
  return metadata;
}


KafkaStreamConfig::KafkaStreamConfig()
{
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

  int32_t i {0};
  SettingMeta root(r, SettingType::stem, "Kafka topic configuration");
  root.set_enum(i++, r + "/KafkaTopic");
  root.set_enum(i++, r + "/KafkaFF");
  root.set_enum(i++, r + "/KafkaMaxBacklog");
  add_definition(root);
}

Setting KafkaStreamConfig::settings() const
{
  std::string r{plugin_name()};
  auto set = get_rich_setting(r);
  set.set(Setting::text(r + "/KafkaTopic", kafka_topic_name_));
  set.set(Setting::boolean(r + "/KafkaFF", kafka_ff_));
  set.set(Setting::integer(r + "/KafkaMaxBacklog", kafka_max_backlog_));
  return set;
}

void KafkaStreamConfig::settings(const Setting& set)
{
  std::string r{plugin_name()};
  kafka_topic_name_ = set.find({r + "/KafkaTopic"}).get_text();
  kafka_ff_ = set.find({r + "/KafkaFF"}).get_bool();
  kafka_max_backlog_ = set.find({r + "/KafkaMaxBacklog"}).get_int();
}
