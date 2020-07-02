

#pragma once

#include <Configuration.h>
#include <librdkafka/rdkafkacpp.h>

class ESSConsumer {
public:
  ESSConsumer(Configuration &Config);

  RdKafka::Message *consume();

  RdKafka::KafkaConsumer *subscribeTopic() const;

  bool handleMessage(RdKafka::Message *message, void *opaque);

  static std::string randomGroupString(size_t length);

private:
  RdKafka::Conf *mConf;
  RdKafka::Conf *mTConf;
  int32_t mPartition0{0};
  RdKafka::KafkaConsumer *mConsumer;
  RdKafka::Topic *mTopic;

  Configuration &mConfig;

  struct Stat {
    uint64_t MessagesRx{0};
    uint64_t MessagesTMO{0};
    uint64_t MessagesData{0};
    uint64_t MessagesEOF{0};
    uint64_t MessagesUnknown{0};
    uint64_t MessagesOther{0};
  } mKafkaStats;
};
