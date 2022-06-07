// Copyright (C) 2020 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file ESSConsumer.cpp
///
//===----------------------------------------------------------------------===//

#include <ESSConsumer.h>
#include <algorithm>
#include <fmt/format.h>
#include <iostream>
#include <unistd.h>
#include <vector>

ESSConsumer::ESSConsumer(std::string Broker, std::string Topic) :
  Topic(Topic), Broker(Broker) {

  mConsumer = subscribeTopic();
  assert(mConsumer != nullptr);
}

RdKafka::KafkaConsumer *ESSConsumer::subscribeTopic() const {
  auto mConf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

  if (!mConf) {
    fmt::print("Unable to create global Conf object\n");
    return nullptr;
  }

  std::string ErrStr;
  /// \todo figure out good values for these
  /// \todo some may be obsolete
  mConf->set("metadata.broker.list", Broker, ErrStr);
  mConf->set("message.max.bytes", MessageMaxBytes, ErrStr);
  mConf->set("fetch.message.max.bytes", FetchMessagMaxBytes, ErrStr);
  mConf->set("replica.fetch.max.bytes", ReplicaFetchMaxBytes, ErrStr);
  std::string GroupId = randomGroupString(16);
  mConf->set("group.id", GroupId, ErrStr);
  mConf->set("enable.auto.commit", EnableAutoCommit, ErrStr);
  mConf->set("enable.auto.offset.store", EnableAutoOffsetStore, ErrStr);

  auto ret = RdKafka::KafkaConsumer::create(mConf, ErrStr);
  if (!ret) {
    fmt::print("Failed to create consumer: {}\n", ErrStr);
    return nullptr;
  }
  //
  // // Start consumer for topic+partition at start offset
  RdKafka::ErrorCode resp = ret->subscribe({Topic});
  if (resp != RdKafka::ERR_NO_ERROR) {
    fmt::print("Failed to subscribe consumer to '{}': {}\n", Topic, err2str(resp));
  }

  return ret;
}


bool ESSConsumer::isNeutronEvent(const EventMessage * EvMsg) {
  auto PixelIds = EvMsg->detector_id();
  auto TOFs = EvMsg->time_of_flight();

  bool failed = false;

  if (PixelIds->size() != TOFs->size()) {
    printf("  isNeutronEvent(): Pixel and TOF arrays differ in size\n");
    failed = true;
  }

  return (not failed);
}

bool ESSConsumer::isMonitor(const EventMessage * EvMsg) {
  std::string SourceName = EvMsg->source_name()->str();
  auto PixelIds = EvMsg->detector_id();
  auto TOFs = EvMsg->time_of_flight();

  bool failed = false;

  if (SourceName.find("mon") == std::string::npos) {
    printf("  isMonitor(): SourceName doesn't contain 'mon'\n");
    failed = true;
  }

  if (PixelIds->size() != 0) {
    printf("  isMonitor(): PixelId's have nonzero size\n");
    failed = true;
  }
  return (not failed);
}

uint32_t ESSConsumer::processEV42Data(RdKafka::Message *Msg) {
  const EventMessage * EvMsg = GetEventMessage(Msg->payload());

  if (isMonitor(EvMsg)) {
    printf("Beam Monitor Event\n");
  } else if (isNeutronEvent(EvMsg)) {
    printf("Neutron Event\n");
  } else {
    printf("Unknown event\n");
  }

  return 0;
}

bool ESSConsumer::handleMessage(RdKafka::Message *Message) {
  mKafkaStats.MessagesRx++;

  switch (Message->err()) {
  case RdKafka::ERR__TIMED_OUT:
    mKafkaStats.MessagesTMO++;
    return false;
    break;

  case RdKafka::ERR_NO_ERROR:
    mKafkaStats.MessagesData++;
    processEV42Data(Message);
    return true;
    break;

  case RdKafka::ERR__PARTITION_EOF:
    mKafkaStats.MessagesEOF++;
    return false;
    break;

  case RdKafka::ERR__UNKNOWN_TOPIC:
  case RdKafka::ERR__UNKNOWN_PARTITION:
    mKafkaStats.MessagesUnknown++;
    fmt::print("Consume failed: {}\n", Message->errstr());
    return false;
    break;

  default: // Other errors
    mKafkaStats.MessagesOther++;
    fmt::print("Consume failed: {}", Message->errstr());
    return false;
  }
}

// Copied from daquiri - added seed based on pid
std::string ESSConsumer::randomGroupString(size_t length) {
  srand(getpid());
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

/// \todo is timeout reasonable?
RdKafka::Message *ESSConsumer::consume() { return mConsumer->consume(1000); }
