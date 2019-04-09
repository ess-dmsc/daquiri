#pragma once

#include <core/plugin/plugin.h>

#pragma GCC diagnostic push
#if defined(__GNUC__) && (__GNUC__ >= 7)
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#endif
#include <librdkafka/rdkafkacpp.h>
#pragma GCC diagnostic pop

namespace Kafka
{
  struct Offsets
  {
    int64_t lo {0};
    int64_t hi {0};
  };

  class Message
  {
    public:
      Message(RdKafka::Message* ptr)
          : low_level(ptr) {}

      std::string print_data() const;

      std::shared_ptr<RdKafka::Message> low_level;
  };

  using MessagePtr = std::shared_ptr<Message>;

  class Consumer
  {
    public:
      Consumer(RdKafka::KafkaConsumer* ptr)
          : low_level(ptr) {}

      std::shared_ptr<RdKafka::KafkaConsumer> low_level;

      MessagePtr consume(uint16_t timeout_ms);
      Offsets get_watermark_offsets(const std::string& topic, int32_t partition);
      Offsets get_watermark_offsets(MessagePtr);

      void seek(const std::string& topic, int32_t partition,
                int64_t offset, int timeout_ms);
      void seek(MessagePtr msg, int64_t offset, int timeout_ms);

  };

  using ConsumerPtr = std::shared_ptr<Consumer>;
}

class KafkaConfigPlugin : public DAQuiri::Plugin
{
  public:
    KafkaConfigPlugin();

    std::string plugin_name() const override { return "KafkaConfig"; }
    DAQuiri::Setting settings() const override;
    void settings(const DAQuiri::Setting&) override;

    Kafka::ConsumerPtr subscribe_topic(std::string topic) const;
    void decomission() const;
    std::vector<RdKafka::TopicPartition *> get_partitions(std::shared_ptr<RdKafka::KafkaConsumer>, std::string topic);
    std::unique_ptr<RdKafka::Metadata> get_kafka_metadata(std::shared_ptr<RdKafka::KafkaConsumer>) const;

    //  private:
    std::string kafka_broker_name_;
    uint16_t kafka_timeout_{1000};
    uint16_t kafka_decomission_wait_{5000};

    static std::string random_string( size_t length );
};


class KafkaStreamConfig : public DAQuiri::Plugin
{
  public:
    KafkaStreamConfig();

    std::string plugin_name() const override { return "KafkaTopicConfig"; }
    DAQuiri::Setting settings() const override;
    void settings(const DAQuiri::Setting&) override;

    // topic-level params
    std::string kafka_topic_name_;
    bool kafka_ff_{false};
    int64_t kafka_max_backlog_{3};
};
