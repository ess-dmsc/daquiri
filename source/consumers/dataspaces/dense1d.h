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

    void reserve(const Coords&) override;
    void clear() override;
    void add(const Entry&) override;
    void add_one(const Coords&) override;
    PreciseFloat get(const Coords&) const override;
    EntryList range(std::vector<Pair> list) const override;
    void recalc_axes() override;

    void save(std::ostream& os) override;

#ifdef DAQUIRI_USE_H5
    void save(hdf5::node::Group&) const override;
    void load(hdf5::node::Group&) override;
#endif

  protected:
    // data
    std::vector<PreciseFloat> spectrum_;
    PreciseFloat total_count_ {0};
    size_t maxchan_ {0};

    std::string data_debug(const std::string& prepend) const override;
};

}
