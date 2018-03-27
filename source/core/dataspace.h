#pragma once

#include <initializer_list>
#include "precise_float.h"
#include "calibration.h"
#include "thread_wrappers.h"

#include <h5cpp/hdf5.hpp>

namespace DAQuiri {

using Pair = std::pair<size_t, size_t>;

using Coords = std::vector<size_t>;
using Entry = std::pair<Coords, PreciseFloat>;
using EntryList_t = std::list<Entry>;
using EntryList = std::shared_ptr<EntryList_t>;

class Dataspace;
using DataspacePtr = std::shared_ptr<Dataspace>;

struct DataAxis
{
  DataAxis() {}
  DataAxis(Calibration c, int16_t resample_shift = 0);
  DataAxis(Calibration c, std::vector<double> dom);

  void expand_domain(size_t ubound);

  std::string label() const;
  std::string debug() const;
  Pair bounds() const;

  friend void to_json(nlohmann::json &j, const DataAxis &s);
  friend void from_json(const nlohmann::json &j, DataAxis &s);

  Calibration calibration;
  int16_t resample_shift_{0};
  std::vector<double> domain;
};

class Dataspace
{
  private:
    std::vector<DataAxis> axes_;
    uint16_t dimensions_{0};

  public:
    Dataspace();
    Dataspace(uint16_t dimensions);
    Dataspace(const Dataspace &other);
    virtual Dataspace *clone() const = 0;
    virtual ~Dataspace() {}

    //get count at coordinates in n-dimensional list
    virtual PreciseFloat get(const Coords &) const = 0;
    //parameters take dimensions_ of ranges (inclusive)
    //optimized retrieval of bulk data as list of Entries
    virtual EntryList range(std::vector<Pair> ranges = {}) const = 0;
    EntryList all_data() const;

    virtual void clear() = 0;
    virtual void reserve(const Coords &) {}
    virtual void add(const Entry &) = 0;
    virtual void add_one(const Coords &) = 0;
    virtual void recalc_axes() = 0;

    virtual void save(std::ostream &) {} //should be pure virtual!

    void load(hdf5::node::Group &);
    void save(hdf5::node::Group &) const;

    //retrieve axis-values for given dimension (can be precalculated energies)
    uint16_t dimensions() const;
    virtual DataAxis axis(uint16_t dimension) const;
    virtual void set_axis(size_t dim, const DataAxis &ax);

    std::string debug(std::string prepend = "") const;

  protected:

    virtual std::string data_debug(const std::string &prepend) const;
    virtual void data_load(hdf5::node::Group) = 0;
    virtual void data_save(hdf5::node::Group) const = 0;
};

}
