#pragma once

#include <initializer_list>
#include "H5CC_Group.h"
#include "precise_float.h"
#include "calibration.h"

#include "thread_wrappers.h"

namespace DAQuiri {

using Entry = std::pair<std::vector<size_t>, PreciseFloat>;
using EntryList_t = std::list<Entry>;
using EntryList = std::shared_ptr<EntryList_t>;

using Pair = std::pair<size_t, size_t>;

struct DataAxis
{
    DataAxis() {}
    DataAxis(Calibration c, size_t resolution);
    DataAxis(Calibration c, size_t resolution, uint16_t bits);

    void expand_domain(size_t ubound);
    void expand_domain(size_t ubound, uint16_t bits);

    std::string label() const;
    std::string debug() const;
    Pair bounds() const;

    Calibration calibration;
    std::vector<double> domain;
};

class Dataspace
{
private:
  std::vector<DataAxis> axes_;
  uint16_t dimensions_ {0};

public:
  Dataspace();
  Dataspace(uint16_t dimensions);
  Dataspace(const Dataspace& other);
  virtual Dataspace* clone() const = 0;
  virtual ~Dataspace() {}

  virtual void add(const Entry&) = 0;
  virtual void add_one(size_t val) {};
  virtual void add_one(size_t val1, size_t val2) {};
  //get count at coordinates in n-dimensional list
  virtual PreciseFloat get(std::initializer_list<size_t> list = {}) const = 0;
  //parameters take dimensions_ of ranges (inclusive)
  //optimized retrieval of bulk data as list of Entries
  virtual EntryList range(std::initializer_list<Pair> list = {}) const = 0;
  virtual void recalc_axes(uint16_t bits) = 0;
  virtual void clear() = 0;

  virtual void load(H5CC::Group&) = 0;
  virtual void save(H5CC::Group&) const = 0;

  //retrieve axis-values for given dimension (can be precalculated energies)
  uint16_t dimensions() const;
  virtual DataAxis axis(uint16_t dimension) const;
  virtual void set_axis(size_t dim, const DataAxis& ax);

  std::string debug(std::string prepend = "") const;

protected:

  virtual std::string data_debug(const std::string& prepend) const;
};

using DataspacePtr = std::shared_ptr<Dataspace>;

}
