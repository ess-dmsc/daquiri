#pragma once

#include <core/consumer.h>
#include <core/util/unique_mangle.h>

namespace DAQuiri
{

class ConsumerFactory
{
 public:
  static ConsumerFactory& singleton()
  {
    static ConsumerFactory singleton_instance;
    return singleton_instance;
  }

  void register_type(ConsumerMetadata tt,
                     std::function<Consumer*(void)> constructor);
  std::vector<std::string> types() const;

  ConsumerPtr create_type(std::string type) const;
  ConsumerPtr create_from_prototype(const ConsumerMetadata& tem) const;
  ConsumerPtr create_from_h5(hdf5::node::Group& group, bool withdata = true) const;
  ConsumerPtr create_copy(ConsumerPtr other) const;

  ConsumerMetadata create_prototype(std::string type) const;

  void clear();

 private:
  std::map<std::string, std::function<Consumer*(void)>> constructors_;
  std::map<std::string, ConsumerMetadata> prototypes_;

  //singleton assurance
  ConsumerFactory() {}
  ConsumerFactory(ConsumerFactory const&);
  void operator=(ConsumerFactory const&);
};

template<class T>
class ConsumerRegistrar
{
 public:
  ConsumerRegistrar()
  {
    ConsumerFactory::singleton().register_type(T().metadata(),
                                               [](void) -> Consumer* { return new T(); });
  }
};

#define DAQUIRI_REGISTER_CONSUMER(T) static DAQuiri::ConsumerRegistrar< T >\
  UNIQUE_MANGLE(MangledDAQuiriConsumerReg) ;

}
