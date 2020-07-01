

#pragma once

#include <librdkafka/rdkafkacpp.h>


class ESSConsumer {
public:
  ESSConsumer();

  void consume();

private:
  std::string ErrStr;
  RdKafka::Conf *Conf;
  RdKafka::Conf *TConf;

  // std::string Topic;
  // std::string Broker;
    // Queue * Qp;
  // int Timeout{1000}; // ms
};
