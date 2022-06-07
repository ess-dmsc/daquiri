// Copyright (C) 2020 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file ESSConsumer.h
///
/// \brief Wrapper class for librdkafka
///
/// Sets up the kafka consumer and handles binning of event pixel ids
//===----------------------------------------------------------------------===//

#pragma once

#include "ev42_events_generated.h"
#include <librdkafka/rdkafkacpp.h>

class ESSConsumer {
public:
  /// \brief Constructor needs the configured Broker and Topic
  ESSConsumer(std::string Broker, std::string Topic);

  /// \brief wrapper function for librdkafka consumer
  RdKafka::Message *consume();

  /// \brief setup librdkafka parameters for Broker and Topic
  RdKafka::KafkaConsumer *subscribeTopic() const;

  /// \brief initial checks for kafka error messages
  /// \return true if message contains data, false otherwise
  bool handleMessage(RdKafka::Message *Msg);

  /// \brief histograms the event pixelids and ignores TOF
  uint32_t processEV42Data(RdKafka::Message *Msg);

  // Checks if Msg is of type and flavor Beam Monitor
  bool isMonitor(const EventMessage * Msg);

  // Checks if Msg is of type Neutron Event
  bool isNeutronEvent(const EventMessage * Msg);

  /// \brief return a random group id so that simultaneous consume from
  /// multiple applications is possible.
  static std::string randomGroupString(size_t length);


private:
  std::string Broker{""};
  std::string Topic{""};
  std::string MessageMaxBytes{"10000000"};
  std::string FetchMessagMaxBytes{"10000000"};
  std::string ReplicaFetchMaxBytes{"10000000"};
  std::string EnableAutoCommit{"false"};
  std::string EnableAutoOffsetStore{"false"};

  RdKafka::Conf *mConf;
  RdKafka::Conf *mTConf;
  RdKafka::KafkaConsumer *mConsumer;
  RdKafka::Topic *mTopic;

  /// \brief Some stat counters
  /// \todo use or delete?
  struct Stat {
    uint64_t MessagesRx{0};
    uint64_t MessagesTMO{0};
    uint64_t MessagesData{0};
    uint64_t MessagesEOF{0};
    uint64_t MessagesUnknown{0};
    uint64_t MessagesOther{0};
  } mKafkaStats;
};
