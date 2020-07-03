

#pragma once

#include <string>

class Configuration {
public:
  Configuration(){};

  Configuration(int XDim, int YDim, std::string Topic, std::string Broker)
      : Geometry({XDim, YDim}), Kafka({Topic, Broker}){};

  // Configurable options
  struct {
    int XDim{256};
    int YDim{256};
  } Geometry;

  struct {
    std::string Topic{"NMX_detector"};
    std::string Broker{"172.17.5.38:9092"};
  } Kafka;

  struct {
    bool Interpolate{false};
    std::string Title;
  } Plot;
};
