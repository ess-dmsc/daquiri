/* Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file      */
//===----------------------------------------------------------------------===//
///
/// \file Configuration.h
///
/// \brief Daquiri light configuration parameters
///
/// Provides some defauls values and allow loading from json file
//===----------------------------------------------------------------------===//

#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include <string>

class Configuration {
public:
  /// \brief constructor using default values
  /// Default are likely to be unsuitable and this should probably
  /// always be followed by a call to fromJsonFile()
  Configuration(){};

  /// \brief loads configuration from json file
  void fromJsonFile(std::string fname);

  /// \brief prints the settings
  void print();

  // Configurable options
  struct {
    int XDim{256};
    int YDim{256};
    int ZDim{1};
  } Geometry;

  struct {
    std::string Topic{"NMX_detector"};
    std::string Broker{"172.17.5.38:9092"};
    std::string MessageMaxBytes{"10000000"};
    std::string FetchMessagMaxBytes{"10000000"};
    std::string ReplicaFetchMaxBytes{"10000000"};
    std::string EnableAutoCommit{"false"};
    std::string EnableAutoOffsetStore{"false"};
    std::string OffsetStoreMethod{"none"};
  } Kafka;

  struct {
    bool ClearPeriodic{false};
    uint32_t ClearEverySeconds{5};
    bool Interpolate{false};
    bool InvertGradient{false};
    std::string Title;
  } Plot;
};
