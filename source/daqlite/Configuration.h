

#pragma once

#include <string>

class Configuration {
public:
  Configuration(int XDim, int YDim, std::string Topic)
       :  mXDim(XDim), mYDim(YDim), mTopic(Topic) {};

  int mXDim {256};
  int mYDim {256};
  std::string mTopic {"NMX_detector"};
};
