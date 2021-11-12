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
    unsigned int Scale{1000};     // ns -> us
    unsigned int MaxValue{25000}; // us
    unsigned int BinSize{512};    // bins

    bool AutoScale{on}; // currently not settable by json
  } TOF;

  struct {
    int XDim{1};
    int YDim{1};
    int ZDim{1};
    int Offset{0};
  } Geometry;

  struct {
    std::string Topic{"nmx_detector"};
    std::string Broker{"172.17.5.38:9092"};
    std::string MessageMaxBytes{"10000000"};
    std::string FetchMessagMaxBytes{"10000000"};
    std::string ReplicaFetchMaxBytes{"10000000"};
    std::string EnableAutoCommit{"false"};
    std::string EnableAutoOffsetStore{"false"};
  } Kafka;

  struct {
    std::string PlotType{"pixels"}; // "tof" is the alternative
    bool ClearPeriodic{false};
    uint32_t ClearEverySeconds{5};
    bool Interpolate{false};
    std::string ColorGradient{"hot"};
    bool InvertGradient{false};
    bool LogScale{false};
    std::string WindowTitle{"Daquiri Lite - Daqlite"};
    std::string PlotTitle{""};
    std::string XAxis{""};
    int Width{600}; // default window width
    int Height{400}; // default window height
  } Plot;
};
