// Copyright (C) 2020 - 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file Configuration.h
///
/// \brief Daquiri light configuration parameters
///
/// Provides some defauls values and allow loading from json file
//===----------------------------------------------------------------------===//

#pragma once

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

class Configuration {
public:
  /// \brief constructor using default values
  /// Default are likely to be unsuitable and this should probably
  /// always be followed by a call to fromJsonFile()
  Configuration(){};

  /// \brief loads configuration from json file
  void fromJsonFile(std::string fname);

  // get the Kafka related config options
  void getKafkaConfig();

  // get the Geometry related config options
  void getGeometryConfig();

  // get the Plot related config options
  void getPlotConfig();

  // get the TOF related config options
  void getTOFConfig();

  /// \brief prints the settings
  void print();

  /// \brief return value of type T from the json object, possibly default,
  // and optionally throws if value is not found
  template <typename T>
  T getVal(std::string Group, std::string Option, T Default,
           bool Throw = false);

  // Configurable options
  struct TOF {
    unsigned int Scale{1000};     // ns -> us
    unsigned int MaxValue{25000}; // us
    unsigned int BinSize{512};    // bins
    bool AutoScaleX{true};
    bool AutoScaleY{true};
  };

  struct Geometry {
    int XDim{1};
    int YDim{1};
    int ZDim{1};
    int Offset{0};
  };

  struct Kafka {
    std::string Topic{"nmx_detector"};
    std::string Broker{"172.17.5.38:9092"};
    std::string MessageMaxBytes{"10000000"};
    std::string FetchMessagMaxBytes{"10000000"};
    std::string ReplicaFetchMaxBytes{"10000000"};
    std::string EnableAutoCommit{"false"};
    std::string EnableAutoOffsetStore{"false"};
  };

  struct Plot {
    std::string PlotType{"pixels"}; // "tof" and "tof2d" are also possible
    bool ClearPeriodic{false};
    uint32_t ClearEverySeconds{5};
    bool Interpolate{false};
    std::string ColorGradient{"hot"};
    bool InvertGradient{false};
    bool LogScale{false};
    std::string WindowTitle{"Daquiri Lite - Daqlite"};
    std::string PlotTitle{""};
    std::string XAxis{""};
    int Width{600};  // default window width
    int Height{400}; // default window height
  };

//
  struct TOF TOF;
  struct Geometry Geometry;
  struct Kafka Kafka;
  struct Plot Plot;

  nlohmann::json JsonObj;
};
