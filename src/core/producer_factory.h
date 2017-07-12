#pragma once

#include <memory>
#include "producer.h"

namespace DAQuiri {

class ProducerFactory {
public:
  static ProducerFactory& singleton()
  {
    static ProducerFactory singleton_instance;
    return singleton_instance;
  }

  ProducerPtr create_type(std::string type, const json &profile);

  void register_type(std::string name, std::function<Producer*(void)> typeConstructor);
  const std::vector<std::string> types();

private:
  std::map<std::string, std::function<Producer*(void)>> constructors;

  //singleton assurance
  ProducerFactory() {}
  ProducerFactory(ProducerFactory const&);
  void operator=(ProducerFactory const&);
};

template<class T>
class ProducerRegistrar {
public:
  ProducerRegistrar(std::string)
  {
    ProducerFactory::singleton().register_type(T::plugin_name(),
                                               [](void) -> Producer * { return new T();});
  }
};

}
