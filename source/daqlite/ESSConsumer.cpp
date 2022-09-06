// Copyright (C) 2020 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file ESSConsumer.cpp
///
//===----------------------------------------------------------------------===//

#include "ev42_events_generated.h"
#include "ev44_events_generated.h"
#include <ESSConsumer.h>
#include <algorithm>
#include <fmt/format.h>
#include <iostream>
#include <unistd.h>
#include <vector>

ESSConsumer::ESSConsumer(Configuration &Config) : mConfig(Config) {
  auto &geom = mConfig.Geometry;
  uint32_t NumPixels = geom.XDim * geom.YDim * geom.ZDim;
  mMinPixel = geom.Offset + 1;
  mMaxPixel = geom.Offset +  NumPixels;
  assert(mMaxPixel != 0);
  assert(mMinPixel < mMaxPixel);
  mHistogram.resize(NumPixels);
  mHistogramTof.resize(mConfig.TOF.BinSize);

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
  mConf->set("metadata.broker.list", mConfig.Kafka.Broker, ErrStr);
  mConf->set("message.max.bytes", mConfig.Kafka.MessageMaxBytes, ErrStr);
  mConf->set("fetch.message.max.bytes", mConfig.Kafka.FetchMessagMaxBytes,
             ErrStr);
  mConf->set("replica.fetch.max.bytes", mConfig.Kafka.ReplicaFetchMaxBytes,
             ErrStr);
  std::string GroupId = randomGroupString(16);
  mConf->set("group.id", GroupId, ErrStr);
  mConf->set("enable.auto.commit", mConfig.Kafka.EnableAutoCommit, ErrStr);
  mConf->set("enable.auto.offset.store", mConfig.Kafka.EnableAutoOffsetStore,
             ErrStr);

  auto ret = RdKafka::KafkaConsumer::create(mConf, ErrStr);
  if (!ret) {
    fmt::print("Failed to create consumer: {}\n", ErrStr);
    return nullptr;
  }
  //
  // // Start consumer for topic+partition at start offset
  RdKafka::ErrorCode resp = ret->subscribe({mConfig.Kafka.Topic});
  if (resp != RdKafka::ERR_NO_ERROR) {
    fmt::print("Failed to subscribe consumer to '{}': {}\n",
               mConfig.Kafka.Topic, err2str(resp));
  }

  return ret;
}

uint32_t ESSConsumer::processEV44Data(RdKafka::Message *Msg) {
  auto EvMsg = GetEvent44Message(Msg->payload());
  auto PixelIds = EvMsg->pixel_id();
  auto TOFs = EvMsg->time_of_flight();

  if (PixelIds->size() != TOFs->size()) {
    return 0;
  }

  for (uint i = 0; i < PixelIds->size(); i++) {
    uint32_t Pixel = (*PixelIds)[i];
    uint32_t Tof = (*TOFs)[i] / mConfig.TOF.Scale; // ns to us

    // accumulate events for 2D TOF
    uint32_t TofBin = std::min(Tof, mConfig.TOF.MaxValue) * (mConfig.TOF.BinSize - 1) / mConfig.TOF.MaxValue;
    mPixelIDs.push_back(Pixel);
    mTOFs.push_back(TofBin);

    if ((Pixel > mMaxPixel) or (Pixel < mMinPixel)) {
      EventDiscard++;
    } else {
      EventAccept++;
      Pixel = Pixel - mConfig.Geometry.Offset;
      mHistogram[Pixel]++;
      Tof = std::min(Tof, mConfig.TOF.MaxValue);
      mHistogramTof[Tof * (mConfig.TOF.BinSize - 1)/ mConfig.TOF.MaxValue]++;
    }
  }
  EventCount += PixelIds->size();
  return PixelIds->size();
}

uint32_t ESSConsumer::processEV42Data(RdKafka::Message *Msg) {
  auto EvMsg = GetEventMessage(Msg->payload());
  auto PixelIds = EvMsg->detector_id();
  auto TOFs = EvMsg->time_of_flight();

  if (PixelIds->size() != TOFs->size()) {
    return 0;
  }

  for (uint i = 0; i < PixelIds->size(); i++) {
    uint32_t Pixel = (*PixelIds)[i];
    uint32_t Tof = (*TOFs)[i] / mConfig.TOF.Scale; // ns to us

    // accumulate events for 2D TOF
    uint32_t TofBin = std::min(Tof, mConfig.TOF.MaxValue) * (mConfig.TOF.BinSize - 1) / mConfig.TOF.MaxValue;
    mPixelIDs.push_back(Pixel);
    mTOFs.push_back(TofBin);

    if ((Pixel > mMaxPixel) or (Pixel < mMinPixel)) {
      EventDiscard++;
    } else {
      EventAccept++;
      Pixel = Pixel - mConfig.Geometry.Offset;
      mHistogram[Pixel]++;
      Tof = std::min(Tof, mConfig.TOF.MaxValue);
      mHistogramTof[Tof * (mConfig.TOF.BinSize - 1)/ mConfig.TOF.MaxValue]++;
    }
  }
  EventCount += PixelIds->size();
  return PixelIds->size();
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
