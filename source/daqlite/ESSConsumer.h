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

#include <Configuration.h>
#include <librdkafka/rdkafkacpp.h>

class ESSConsumer {
public:
  /// \brief Constructor needs the configured Broker and Topic
  ESSConsumer(Configuration &Config);

  /// \brief wrapper function for librdkafka consumer
  RdKafka::Message *consume();

  /// \brief setup librdkafka parameters for Broker and Topic
  RdKafka::KafkaConsumer *subscribeTopic() const;

  /// \brief initial checks for kafka error messages
  /// \return true if message contains data, false otherwise
  bool handleMessage(RdKafka::Message *message);

  /// \brief histograms the event pixelids and ignores TOF
  uint32_t processEV42Data(RdKafka::Message *Msg);

  /// \brief histograms the event pixelids and ignores TOF
  uint32_t processEV44Data(RdKafka::Message *Msg);

  /// \brief return a random group id so that simultaneous consume from
  /// multiple applications is possible.
  static std::string randomGroupString(size_t length);

  // Histogram(s) and counts
  std::vector<uint32_t> mHistogramPlot;
  std::vector<uint32_t> mHistogram;
  std::vector<uint32_t> mHistogramTofPlot;
  std::vector<uint32_t> mHistogramTof;

  std::vector<uint32_t> mPixelIDs;
  std::vector<uint32_t> mPixelIDsPlot;
  std::vector<uint32_t> mTOFs;
  std::vector<uint32_t> mTOFsPlot;

  uint64_t EventCount{0};
  uint64_t EventAccept{0};
  uint64_t EventDiscard{0};



private:
  RdKafka::Conf *mConf;
  RdKafka::Conf *mTConf;
  RdKafka::KafkaConsumer *mConsumer;
  RdKafka::Topic *mTopic;

  /// \brief configuration obtained from main()
  Configuration &mConfig;

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

  uint32_t mMinPixel{0}; ///< Offset
  uint32_t mMaxPixel{0}; ///< Number of pixels + offset
};
