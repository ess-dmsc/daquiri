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
