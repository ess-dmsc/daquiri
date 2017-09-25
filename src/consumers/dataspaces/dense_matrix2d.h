#pragma once

#include "dataspace.h"
#include <Eigen/Sparse>

namespace DAQuiri
{

class DenseMatrix2D : public Dataspace
{
  public:
    DenseMatrix2D();
    DenseMatrix2D* clone() const override
    { return new DenseMatrix2D(*this); }

    void reserve(const Coords&) override;
    void clear() override;
    void add(const Entry&) override;
    void add_one(const Coords&) override;
    PreciseFloat get(const Coords&) const override;
    EntryList range(std::vector<Pair> list) const override;
    void recalc_axes() override;

    void save(H5CC::Group&) const override;
    void load(H5CC::Group&) override;
    std::string data_debug(const std::string& prepend) const override;

  protected:
    typedef Eigen::Matrix<uint64_t, Eigen::Dynamic, Eigen::Dynamic> data_type_t;

    //the data itself
    data_type_t spectrum_;
    uint64_t total_count_ {0};

    Coords limits_ {0,0};

    inline void adjust_maxima(const uint16_t& x, const uint16_t& y)
    {
      if (x > limits_[0])
        limits_[0] = x;
      if (y > limits_[1])
        limits_[1] = y;
      if ((spectrum_.rows() <= limits_[0]) || (spectrum_.cols() <= limits_[1]))
        this->reserve(limits_);
    }

    inline void bin_pair(const uint16_t& x, const uint16_t& y,
                         const PreciseFloat& count)
    {
      adjust_maxima(x,y);
      spectrum_.coeffRef(x, y) += count;
      total_count_ += count;
    }

    inline void bin_one(const uint16_t& x, const uint16_t& y)
    {
      adjust_maxima(x,y);
      spectrum_(x, y) += 1;
      total_count_ ++;
    }

    bool is_symmetric();

    void fill_list(EntryList &result,
                   size_t min0, size_t max0,
                   size_t min1, size_t max1) const;
};

}
