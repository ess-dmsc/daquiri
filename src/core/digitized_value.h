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

#include <string>

namespace DAQuiri {

class DigitizedVal {
public:

  inline DigitizedVal() {}

  inline DigitizedVal(uint16_t v, uint16_t b)
    : val_(v)
    , bits_(b)
  {}

  inline void set_val(uint16_t v)
  {
    val_ = v;
  }

  inline uint16_t bits() const
  {
    return bits_;
  }

  inline uint16_t val(uint16_t bits) const
  {
    if (!bits && bits_)
      return 0;
    else if (!bits_ || (bits == bits_))
      return val_;
    else if (bits < bits_)
      return val_ >> (bits_ - bits);
    else
      return val_ << (bits - bits_);
  }

  inline bool operator==(const DigitizedVal other) const
  {
    return ((val_ == other.val_) && (bits_ == other.bits_));
  }

  inline bool operator!=(const DigitizedVal other) const
  {
    return !operator==(other);
  }

  inline std::string debug() const
  {
    return std::to_string(val_) +
        "(" + std::to_string(bits_) + "b)";
  }

private:
  uint16_t  val_  {0};
  uint16_t  bits_ {0};
};

}
