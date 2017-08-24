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
  mutable mutex_st mutex_;
  std::vector<DataAxis> axes_;
  uint16_t dimensions_ {0};

public:
  Dataspace();
  Dataspace(uint16_t dimensions);
  Dataspace(const Dataspace& other);
  virtual Dataspace* clone() const = 0;
  virtual ~Dataspace() {}

  void add(const Entry&);
  //get count at coordinates in n-dimensional list
  PreciseFloat get(std::initializer_list<size_t> list = {}) const;
  //parameters take dimensions_ of ranges (inclusive)
  //optimized retrieval of bulk data as list of Entries
  EntryList range(std::initializer_list<Pair> list = {}) const;
  void recalc_axes(uint16_t bits);
  void clear();

  void load(H5CC::Group&);
  void save(H5CC::Group&) const;

  //retrieve axis-values for given dimension (can be precalculated energies)
  uint16_t dimensions() const;
  DataAxis axis(uint16_t dimension) const;
  void set_axis(size_t dim, const DataAxis& ax);

  std::string debug(std::string prepend = "") const;

protected:
  uint16_t _dimensions() const;
  DataAxis _axis(uint16_t dimension) const;
  void _set_axis(size_t dim, const DataAxis& ax);

  //////////////////////////////////////////
  //////////THIS IS THE MEAT////////////////
  ///implement these to make custom types///
  //////////////////////////////////////////

  virtual void _add(const Entry&) = 0;
  virtual PreciseFloat _get(std::initializer_list<size_t>) const = 0;
  virtual EntryList _range(std::initializer_list<Pair>) const = 0;
  virtual void _clear() = 0;

  virtual void _recalc_axes(uint16_t bits) = 0;

  virtual void _load(H5CC::Group&) {}
  virtual void _save(H5CC::Group&) const {}
  virtual std::string _data_debug(const std::string& prepend) const;
};

using DataspacePtr = std::shared_ptr<Dataspace>;

}
