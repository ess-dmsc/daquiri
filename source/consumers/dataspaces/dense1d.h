#pragma once

#include "dataspace.h"

namespace DAQuiri
{

class Dense1D : public Dataspace
{
  public:
    Dense1D();
    Dense1D* clone() const override
    { return new Dense1D(*this); }

    bool empty() const override;
    void reserve(const Coords&) override;
    void clear() override;
    void add(const Entry&) override;
    void add_one(const Coords&) override;
    PreciseFloat get(const Coords&) const override;
    EntryList range(std::vector<Pair> list) const override;
    void recalc_axes() override;

    void export_csv(std::ostream& os) const override;

  protected:
    // data
    std::vector<PreciseFloat> spectrum_;
    size_t maxchan_ {0};

    std::string data_debug(const std::string& prepend) const override;
    void data_save(const hdf5::node::Group&) const override;
    void data_load(const hdf5::node::Group&) override;
};

}
