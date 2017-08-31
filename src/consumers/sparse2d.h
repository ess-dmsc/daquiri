#pragma once

#include "dataspace.h"

#include <Eigen/Sparse>

namespace DAQuiri
{

class Sparse2D : public Dataspace
{
  public:
    Sparse2D();
    Sparse2D* clone() const override
    { return new Sparse2D(*this); }

    void clear() override;
    void add(const Entry&) override;
    void add_one(size_t val1, size_t val2) override;
    PreciseFloat get(std::initializer_list<size_t> list) const override;
    EntryList range(std::initializer_list<Pair> list) const override;
    void recalc_axes(uint16_t bits) override;

    void save(H5CC::Group&) const override;
    void load(H5CC::Group&) override;
    std::string data_debug(const std::string& prepend) const override;

  protected:
    //typedef std::map<std::pair<uint16_t,uint16_t>, PreciseFloat> SpectrumMap2D;
    typedef Eigen::SparseMatrix<double> SpectrumMap2D;


    //the data itself
    SpectrumMap2D spectrum_;
    uint64_t total_count_ {0};
    uint16_t max0_ {0};
    uint16_t max1_ {0};

    inline void bin_pair(const uint16_t& x, const uint16_t& y,
                         const PreciseFloat& count) {
      spectrum_.coeffRef(x, y) += count;
      total_count_ += count;
      max0_ = std::max(max0_, x);
      max1_ = std::max(max1_, y);
    }

    bool is_symmetric();

    void fill_list(EntryList &result,
                   size_t min0, size_t max0,
                   size_t min1, size_t max1) const;
};

}
