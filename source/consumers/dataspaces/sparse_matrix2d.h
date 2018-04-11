#pragma once

#include "dataspace.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-in-bool-context"
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <Eigen/Sparse>
#pragma GCC diagnostic pop

namespace DAQuiri
{

class SparseMatrix2D : public Dataspace
{
  public:
    SparseMatrix2D();
    SparseMatrix2D* clone() const override
    { return new SparseMatrix2D(*this); }

    bool empty() const override;
    void reserve(const Coords&) override;
    void clear() override;
    void add(const Entry&) override;
    void add_one(const Coords&) override;
    PreciseFloat get(const Coords&) const override;
    EntryList range(std::vector<Pair> list) const override;
    void recalc_axes() override;

    void save(std::ostream& os) override;

  protected:
    typedef Eigen::SparseMatrix<double> data_type_t;

    //the data itself
    data_type_t spectrum_;

    Coords limits_ {0,0};

    inline void adjust_maxima(const uint16_t& x, const uint16_t& y)
    {
      if (x > limits_[0])
        limits_[0] = x;
      if (y > limits_[1])
        limits_[1] = y;
      if ((spectrum_.rows() <= static_cast<int64_t>(limits_[0])) ||
          (spectrum_.cols() <= static_cast<int64_t>(limits_[1])))
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
      spectrum_.coeffRef(x, y) ++;
      total_count_ ++;
    }

    bool is_symmetric();

    void fill_list(EntryList &result,
                   int64_t min0, int64_t max0,
                   int64_t min1, int64_t max1) const;

    void data_save(hdf5::node::Group) const override;
    void data_load(hdf5::node::Group) override;

    std::string data_debug(const std::string& prepend) const override;
};

}
