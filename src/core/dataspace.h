#pragma once

#include <initializer_list>
#include <boost/thread.hpp>
#include "H5CC_Group.h"
#include "precise_float.h"
#include "calibration.h"

namespace DAQuiri {

using Entry = std::pair<std::vector<size_t>, PreciseFloat>;
using EntryList = std::list<Entry>;
using Pair = std::pair<size_t, size_t>;

struct DataAxis
{
    DataAxis() {}
    DataAxis(Calibration c, size_t resolution);
    DataAxis(Calibration c, size_t resolution, uint16_t bits);

    std::string label() const;
    std::string debug() const;
    Pair bounds() const;

    Calibration calibration;
    std::vector<double> domain;
};

class Dataspace
{
protected:
  mutable boost::shared_mutex mutex_;
  std::vector<DataAxis> axes_;
  uint16_t dimensions_ {0};

public:
  Dataspace();
  Dataspace(uint16_t dimensions);
  Dataspace(const Dataspace& other);
  virtual Dataspace* clone() const = 0;
  virtual ~Dataspace() {}

  void load(H5CC::Group&);
  void save(H5CC::Group&) const;

  //get count at coordinates in n-dimensional list
  //optimized retrieval of bulk data as list of Entries
  //parameters take dimensions_number of ranges (inclusive)
  PreciseFloat data(std::initializer_list<size_t> list = {}) const;
  std::unique_ptr<EntryList> data(std::initializer_list<Pair> list = {}) const;
  void append(const Entry&);

  //retrieve axis-values for given dimension (can be precalculated energies)
  uint16_t dimensions() const;
  DataAxis axis(uint16_t dimension) const;
  void set_axis(size_t dim, const DataAxis& ax);

  std::string debug(std::string prepend = "") const;

protected:
  //////////////////////////////////////////
  //////////THIS IS THE MEAT////////////////
  ///implement these to make custom types///
  //////////////////////////////////////////
  
  virtual void _recalc_axes() = 0;

  virtual PreciseFloat _data(std::initializer_list<size_t>) const;
  virtual std::unique_ptr<EntryList> _data(std::initializer_list<Pair>) const;
  virtual void _append(const Entry&) = 0;

  virtual void _load(H5CC::Group&) {}
  virtual void _save(H5CC::Group&) const {}
  virtual std::string _data_debug(const std::string& prepend) const;
};

using DataspacePtr = std::shared_ptr<Dataspace>;

}
