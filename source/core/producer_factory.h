#pragma once

#include <memory>
#include <core/producer.h>
#include <core/util/unique_mangle.h>

namespace DAQuiri
{

class ProducerFactory
{
 public:
  static ProducerFactory& singleton()
  {
    static ProducerFactory singleton_instance;
    return singleton_instance;
  }

  void register_type(std::string name,
                     std::function<Producer*(void)> constructor);

  std::vector<std::string> types() const;
  ProducerPtr create_type(std::string type) const;
  Setting default_settings(std::string type) const;

  void clear();

 private:
  std::map<std::string, std::function<Producer*(void)>> constructors_;

  //singleton assurance
  ProducerFactory() {}
  ProducerFactory(ProducerFactory const&);
  void operator=(ProducerFactory const&);
};

template<class T>
class ProducerRegistrar
{
 public:
  ProducerRegistrar()
  {
    ProducerFactory::singleton().register_type(T().plugin_name(),
                                               [](void) -> Producer* { return new T(); });
  }
};

#define DAQUIRI_REGISTER_PRODUCER(T) DAQuiri::ProducerRegistrar< T >\
  UNIQUE_MANGLE(MangledDAQuiriProducerReg) ;

}
