
#include <ESSConsumer.h>
#include <iostream>

ESSConsumer::ESSConsumer(Configuration &Config) : mConfig(Config) {
  mConsumer = subscribeTopic();
  assert(mConsumer != nullptr);
}

RdKafka::KafkaConsumer *ESSConsumer::subscribeTopic() const {
  auto mConf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

  if (!mConf) {
    printf("Unable to create global Conf object\n");
    return nullptr;
  }

  printf("used broker name %s\n", mConfig.mBroker.c_str());

  std::string ErrStr;
  mConf->set("metadata.broker.list", mConfig.mBroker, ErrStr);
  mConf->set("message.max.bytes", "10000000", ErrStr);
  mConf->set("fetch.message.max.bytes", "10000000", ErrStr);
  mConf->set("replica.fetch.max.bytes", "10000000", ErrStr);
  mConf->set("group.id", randomGroupString(16), ErrStr);
  mConf->set("enable.auto.commit", "false", ErrStr);
  mConf->set("enable.auto.offset.store", "false", ErrStr);
  mConf->set("offset.store.method", "none", ErrStr);

  // //  mConf->set("auto.offset.reset", "largest", ErrStr);
  // //  mConf->set("session.timeout.ms", "10000", ErrStr);
  // //  mConf->set("api.version.request", "true", ErrStr);
  //
  auto ret = RdKafka::KafkaConsumer::create(mConf, ErrStr);
  if (!ret) {
    printf("Failed to create consumer: %s\n", ErrStr.c_str());
    return nullptr;
  }
  //
  // // Start consumer for topic+partition at start offset
  RdKafka::ErrorCode resp = ret->subscribe({mConfig.mTopic});
  if (resp != RdKafka::ERR_NO_ERROR) {
    printf("Failed to subscribe consumer to '%s': %s", mConfig.mTopic.c_str(),
           err2str(resp).c_str());
  }

  return ret;
}

bool ESSConsumer::handleMessage(RdKafka::Message *message, void *opaque) {
  size_t MessageOffset;
  size_t MessageLength;

  mKafkaStats.MessagesRx++;

  switch (message->err()) {
  case RdKafka::ERR__TIMED_OUT:
    mKafkaStats.MessagesTMO++;
    // printf("ERR__TIMED_OUT\n");
    return false;
    break;

  case RdKafka::ERR_NO_ERROR:
    mKafkaStats.MessagesData++;
    /* Real message */
    MessageOffset = message->offset();
    MessageLength = message->len();
    // printf("Message offset: %zu - length %zu\n", MessageOffset,
    // MessageLength);
    if (message->key()) {
      std::cout << "Key: " << *message->key() << std::endl;
    }
    return true;
    break;

  case RdKafka::ERR__PARTITION_EOF:
    mKafkaStats.MessagesEOF++;
    // printf("ERR__PARTITION_EOF\n");
    return false;
    break;

  case RdKafka::ERR__UNKNOWN_TOPIC:
  case RdKafka::ERR__UNKNOWN_PARTITION:
    mKafkaStats.MessagesUnknown++;
    std::cerr << "Consume failed: " << message->errstr() << std::endl;
    return false;
    break;

  default:
    /* Errors */
    mKafkaStats.MessagesOther++;
    std::cerr << "Consume failed: " << message->errstr() << std::endl;
    return false;
  }
}

// Copied from daquiri
std::string ESSConsumer::randomGroupString(size_t length) {
  auto randchar = []() -> char {
    const char charset[] = "0123456789"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[rand() % max_index];
  };
  std::string str(length, 0);
  std::generate_n(str.begin(), length, randchar);
  return str;
}

RdKafka::Message *ESSConsumer::consume() {
  RdKafka::Message *Msg = mConsumer->consume(1000);
  return Msg;
}
