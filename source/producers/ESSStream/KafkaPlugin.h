#pragma once

#include "plugin.h"
#include <librdkafka/rdkafkacpp.h>

class KafkaConfigPlugin : public DAQuiri::Plugin
{
  public:
    KafkaConfigPlugin();

    std::string plugin_name() const override { return "KafkaConfig"; }
    DAQuiri::Setting settings() const override;
    void settings(const DAQuiri::Setting&) override;

    std::shared_ptr<RdKafka::KafkaConsumer> subscribe_topic(std::string topic) const;
    void decomission() const;
    std::vector<RdKafka::TopicPartition *> get_partitions(std::shared_ptr<RdKafka::KafkaConsumer>, std::string topic);
    std::unique_ptr<RdKafka::Metadata> get_kafka_metadata(std::shared_ptr<RdKafka::KafkaConsumer>) const;
    void seek(std::shared_ptr<RdKafka::KafkaConsumer>,
              const std::string &topic, uint32_t partition, int64_t offset) const;
    std::string debug(std::shared_ptr<RdKafka::Message> kmessage);

    //  private:
    std::string kafka_broker_name_;
    uint16_t kafka_timeout_{1000};
    uint16_t kafka_decomission_wait_{5000};
};
