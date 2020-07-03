
#include <ESSConsumer.h>
#include "ev42_events_generated.h"
#include <iostream>

ESSConsumer::ESSConsumer(Configuration &Config) : mConfig(Config) {
  mMaxPixel = mConfig.Geometry.XDim * mConfig.Geometry.YDim;
  assert(mMaxPixel != 0);
  mHistogram.resize(mMaxPixel);

  mConsumer = subscribeTopic();
  assert(mConsumer != nullptr);
}

RdKafka::KafkaConsumer *ESSConsumer::subscribeTopic() const {
  auto mConf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

  if (!mConf) {
    printf("Unable to create global Conf object\n");
    return nullptr;
  }

  printf("used broker name %s\n", mConfig.Kafka.Broker.c_str());

  std::string ErrStr;
  mConf->set("metadata.broker.list", mConfig.Kafka.Broker, ErrStr);
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
  RdKafka::ErrorCode resp = ret->subscribe({mConfig.Kafka.Topic});
  if (resp != RdKafka::ERR_NO_ERROR) {
    printf("Failed to subscribe consumer to '%s': %s", mConfig.Kafka.Topic.c_str(),
           err2str(resp).c_str());
  }

  return ret;
}

uint32_t ESSConsumer::processEV42Data(RdKafka::Message *Msg) {
    auto EvMsg = GetEventMessage(Msg->payload());
    auto PixelIds = EvMsg->detector_id();
    auto TOFs = EvMsg->time_of_flight();
    if (PixelIds->size() != TOFs->size()) {
      return 0;
    }

    for (const uint32_t & Pixel : *PixelIds) {
      if (Pixel >= mMaxPixel) {
        printf("Error: invalid pixel id: %d > %d\n", Pixel, mMaxPixel);
        exit(0);
      }
      mHistogram[Pixel]++;
    }
    mCounts += PixelIds->size();
    return PixelIds->size();
}

bool ESSConsumer::handleMessage(RdKafka::Message *Message, void *Opaque) {
  size_t MessageOffset;
  size_t MessageLength;

  mKafkaStats.MessagesRx++;

  switch (Message->err()) {
  case RdKafka::ERR__TIMED_OUT:
    mKafkaStats.MessagesTMO++;
    // printf("ERR__TIMED_OUT\n");
    return false;
    break;

  case RdKafka::ERR_NO_ERROR:
    mKafkaStats.MessagesData++;
    /* Real message */
    //MessageOffset = Message->offset();
    //MessageLength = Message->len();
    // printf("Message offset: %zu - length %zu\n", MessageOffset,
    // MessageLength);
    processEV42Data(Message);
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
    std::cerr << "Consume failed: " << Message->errstr() << std::endl;
    return false;
    break;

  default:
    /* Errors */
    mKafkaStats.MessagesOther++;
    std::cerr << "Consume failed: " << Message->errstr() << std::endl;
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
