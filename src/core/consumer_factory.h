#pragma once

#include "consumer.h"
#include "H5CC_Group.h"

namespace DAQuiri {

class ConsumerFactory {
 public:
  static ConsumerFactory& singleton()
  {
    static ConsumerFactory singleton_instance;
    return singleton_instance;
  }

  void register_type(ConsumerMetadata tt, std::function<Consumer*(void)> typeConstructor);
  const std::vector<std::string> types();
  
  ConsumerPtr create_type(std::string type);
  ConsumerPtr create_from_prototype(const ConsumerMetadata& tem);
  ConsumerPtr create_from_h5(H5CC::Group &group, bool withdata = true);
  ConsumerPtr create_copy(ConsumerPtr other);

  ConsumerMetadata create_prototype(std::string type);

 private:
  std::map<std::string, std::function<Consumer*(void)>> constructors;
  std::map<std::string, ConsumerMetadata> prototypes;

  //singleton assurance
  ConsumerFactory() {}
  ConsumerFactory(ConsumerFactory const&);
  void operator=(ConsumerFactory const&);
};

template<class T>
class ConsumerRegistrar {
public:
  ConsumerRegistrar(std::string)
  {
    ConsumerFactory::singleton().register_type(T().metadata(),
                                         [](void) -> Consumer * { return new T();});
  }
};

}
