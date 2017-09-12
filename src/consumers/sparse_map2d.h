#pragma once

#include "dataspace.h"

namespace DAQuiri
{

class SparseMap2D : public Dataspace
{
  public:
    SparseMap2D();
    SparseMap2D* clone() const override
    { return new SparseMap2D(*this); }

    void clear() override;
    void add(const Entry&) override;
    PreciseFloat get(std::initializer_list<size_t> list) const override;
    EntryList range(std::initializer_list<Pair> list) const override;
    void recalc_axes(uint16_t bits) override;

    void save(H5CC::Group&) const override;
    void load(H5CC::Group&) override;
    std::string data_debug(const std::string& prepend) const override;

  protected:
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

    void fill_list(EntryList &result,
                   size_t min0, size_t max0,
                   size_t min1, size_t max1) const;
};

}