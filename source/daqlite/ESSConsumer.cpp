
#include <ESSConsumer.h>
#include <iostream>


ESSConsumer::ESSConsumer() {
  Conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
  TConf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

  RdKafka::Consumer *consumer = RdKafka::Consumer::create(Conf, ErrStr);
  if (!consumer) {
    std::cout << "Failed to create consumer: " << ErrStr << std::endl;
    exit(1);
  }
}


void ESSConsumer::consume() {

}
