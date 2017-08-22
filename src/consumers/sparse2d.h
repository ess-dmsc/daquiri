#pragma once

#include "dataspace.h"

namespace DAQuiri
{

class Sparse2D : public Dataspace
{
public:
  Sparse2D();

protected:
  void _add(const Entry&) override;
  PreciseFloat _get(std::initializer_list<size_t> list) const override;
  std::unique_ptr<EntryList> _range(std::initializer_list<Pair> list) const override;

  void _save(H5CC::Group&) const override;
  void _load(H5CC::Group&) override;
  std::string _data_debug(const std::string& prepend) const override;

  typedef std::map<std::pair<uint16_t,uint16_t>, PreciseFloat> SpectrumMap2D;

  //the data itself
  SpectrumMap2D spectrum_;
  PreciseFloat total_count_ {0};
  uint16_t max0_ {0};
  uint16_t max1_ {0};

  void bin_pair(const uint16_t& x,
                const uint16_t& y,
                const PreciseFloat& count);

  bool is_symmetric();

  void _fill_list(std::unique_ptr<EntryList>& result,
                  size_t min0, size_t max0,
                  size_t min1, size_t max1) const;
};

}