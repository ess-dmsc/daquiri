
#include <ESSConsumer.h>
#include <iostream>


ESSConsumer::ESSConsumer(Configuration & Config)
    : mConfig(Config) {

  mConsumer = subscribeTopic();
  assert(mConsumer != nullptr);

  // mConf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
  // mTConf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
  // assert(mConf != nullptr);
  // assert(mTConf != nullptr);
  //
  //
  // mmConf->set("metadata.broker.list", mConfig.mBroker, mErrStr);
  // mmConf->set("default_topic_conf", mTConf, mErrStr);
  // //delete TConf; ??
  //
  // mConsumer = RdKafka::Consumer::create(mConf, mErrStr);
  // if (!mConsumer) {
  //   std::cout << "Failed to create consumer: " << mErrStr << std::endl;
  //   exit(1);
  // }
  //
  // mTopic = RdKafka::Topic::create(mConsumer, mConfig.mTopic, mTConf, mErrStr);
  // if (!mTopic) {
  //   std::cerr << "Failed to create topic: " << mErrStr << std::endl;
  //   exit(1);
  // }
}

RdKafka::KafkaConsumer * ESSConsumer::subscribeTopic() const {
  auto mConf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

  if (!mConf) {
    printf("<KafkaConfigPlugin> Unable to create global Conf object\n");
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
    printf("Failed to subscribe consumer to '%s': %s",
        mConfig.mTopic.c_str(), err2str(resp).c_str());
  }

  return ret;
}

void ESSConsumer::handleMessage(RdKafka::Message* message, void* opaque) {
  size_t MessageOffset;
  size_t MessageLength;

  switch (message->err()) {
    case RdKafka::ERR__TIMED_OUT:
      //printf("ERR__TIMED_OUT\n");
      break;

    case RdKafka::ERR_NO_ERROR:
      /* Real message */
      MessageOffset = message->offset();
      MessageLength = message->len();
      printf("Message offset: %zu - length %zu\n", MessageOffset, MessageLength);
      if (message->key()) {
        std::cout << "Key: " << *message->key() << std::endl;
      }
      break;

    case RdKafka::ERR__PARTITION_EOF:
      //printf("ERR__PARTITION_EOF\n");
      break;

    case RdKafka::ERR__UNKNOWN_TOPIC:
    case RdKafka::ERR__UNKNOWN_PARTITION:
      std::cerr << "Consume failed: " << message->errstr() << std::endl;
      break;

    default:
      /* Errors */
      std::cerr << "Consume failed: " << message->errstr() << std::endl;
  }
}


std::string ESSConsumer::randomGroupString( size_t length )
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


RdKafka::Message * ESSConsumer::consume() {
    //RdKafka::Message *Msg = mConsumer->consume(mTopic, mPartition0, 1000);
    RdKafka::Message *Msg = mConsumer->consume(1000);
    return Msg;
}
