

#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include <string>

class Configuration {
public:
  Configuration(){};

  Configuration(int XDim, int YDim, std::string Topic, std::string Broker)
      : Geometry({XDim, YDim}), Kafka({Topic, Broker}){};


  void print();

  void fromJsonFile(std::string fname);

  // Configurable options
  struct {
    int XDim{256};
    int YDim{256};
    int ZDim{1};
  } Geometry;

  struct {
    std::string Topic{"NMX_detector"};
    std::string Broker{"172.17.5.38:9092"};
  } Kafka;

  struct {
    bool ClearPeriodic{false};
    uint32_t ClearEverySeconds{5};
    bool Interpolate{false};
    std::string Title;
  } Plot;
};
