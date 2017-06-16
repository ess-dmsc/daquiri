/*******************************************************************************
 *
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 *
 * This software can be redistributed and/or modified freely provided that
 * any derivative works bear some notice that they are derived from it, and
 * any modified versions bear some notice that they have been modified.
 *
 * Author(s):
 *      Martin Shetty (NIST)
 *
 ******************************************************************************/

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
  
  SinkPtr create_type(std::string type);
  SinkPtr create_from_prototype(const ConsumerMetadata& tem);
  SinkPtr create_from_h5(H5CC::Group &group, bool withdata = true);
  SinkPtr create_from_file(std::string filename);
  SinkPtr create_copy(SinkPtr other);

  ConsumerMetadata create_prototype(std::string type);

 private:
  std::map<std::string, std::function<Consumer*(void)>> constructors;
  std::map<std::string, std::string> ext_to_type;
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
