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

#include "setting.h"

namespace DAQuiri {

class Detector
{
private:
  std::string name_ {"unknown"};
  std::string type_ {"unknown"};

  Setting settings_ {"Optimization"};

public:
  Detector();
  Detector(std::string name);
  std::string debug(std::string prepend) const;

  std::string name() const;
  void set_name(const std::string&);

  std::string type() const;
  void set_type(const std::string&);

  void clear_optimizations();
  std::list<Setting> optimizations() const;
  Setting get_setting(std::string id) const;
  void add_optimizations(const std::list<Setting>&,
                         bool writable_only = false);

  bool shallow_equals(const Detector& other) const {return (name_ == other.name_);}
  bool operator== (const Detector& other) const;
  bool operator!= (const Detector& other) const;
  
  friend void to_json(json& j, const Detector &s);
  friend void from_json(const json& j, Detector &s);
  json to_json(bool options) const;
};

}
