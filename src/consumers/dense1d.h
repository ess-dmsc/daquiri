#pragma once

#include "dataspace.h"

namespace DAQuiri
{

class Dense1D : public Dataspace
{
public:
  Dense1D();
  Dense1D* clone() const
  { return new Dense1D(*this); }

protected:
  void _add(const Entry&) override;
  PreciseFloat _get(std::initializer_list<size_t> list) const override;
  EntryList _range(std::initializer_list<Pair> list) const override;

  void _save(H5CC::Group&) const override;
  void _load(H5CC::Group&) override;
  std::string _data_debug(const std::string& prepend) const override;

  // data
  std::vector<PreciseFloat> spectrum_;
  PreciseFloat total_count_ {0};
  size_t maxchan_ {0};
};

}
