

#pragma once

#include <string>

class Configuration {
public:
  Configuration(){};

  Configuration(int XDim, int YDim, std::string Topic, std::string Broker)
      : mXDim(XDim), mYDim(YDim), mTopic(Topic), mBroker(Broker){};

  int mXDim{256};
  int mYDim{256};
  std::string mTopic{"NMX_detector"};
  std::string mBroker{"172.17.5.38:9092"};
};
