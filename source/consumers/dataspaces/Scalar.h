#pragma once

#include <core/Dataspace.h>

namespace DAQuiri {

class Scalar : public Dataspace
{
  public:
    Scalar();

    Scalar* clone() const override { return new Scalar(*this); }

    bool empty() const override;
    void reserve(const Coords&) override;
    void clear() override;
    void add(const Entry&) override;
    void add_one(const Coords&) override;
    PreciseFloat get(const Coords&) const override;
    EntryList range(std::vector<Pair> list) const override;
    void recalc_axes() override;

    void export_csv(std::ostream &) const override;

  protected:
    // data
    PreciseFloat data_ {0};
    bool has_data_ {false};

    PreciseFloat max_val_ {0};
    PreciseFloat min_val_ {0};

    std::string data_debug(const std::string& prepend) const override;
    void data_save(const hdf5::node::Group&) const override;
    void data_load(const hdf5::node::Group&) override;
};

}
