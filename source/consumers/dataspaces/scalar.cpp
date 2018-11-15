#include <consumers/dataspaces/scalar.h>
#include <core/util/h5json.h>

namespace DAQuiri {

Scalar::Scalar()
    : Dataspace(0) {}

bool Scalar::empty() const
{
  return !has_data_;
}

void Scalar::reserve(const Coords&)
{
}

void Scalar::clear()
{
  total_count_ = 0;
  has_data_ = false;
  data_ = 0.0;
  max_val_ = 0.0;
  min_val_ = 0.0;
}

void Scalar::add(const Entry& e)
{
  if ((e.first.size() != dimensions()))
    return;

  data_ = e.second;
  total_count_++;
  if (!has_data_)
  {
    max_val_ = data_;
    min_val_ = data_;
  }
  max_val_ = std::max(max_val_, data_);
  min_val_ = std::min(min_val_, data_);
  has_data_ = true;
}

void Scalar::add_one(const Coords& coords)
{
  if (coords.size() != dimensions())
    return;

  data_++;
  total_count_++;
  if (!has_data_)
  {
    max_val_ = data_;
    min_val_ = data_;
  }
  max_val_ = std::max(max_val_, data_);
  min_val_ = std::min(min_val_, data_);
  has_data_ = true;
}

void Scalar::recalc_axes()
{
#if 0
  //TODO: bad things in case of 0 or large values, think of something better
  auto ax = axis(0);
  ax.expand_domain(max_val_);
  set_axis(0, ax);
#endif
}

PreciseFloat Scalar::get(const Coords&) const
{
  return data_;
}

EntryList Scalar::range(std::vector<Pair>) const
{
  EntryList result(new EntryList_t);
  if (has_data_)
  {
    result->push_back({{}, min_val_});
    result->push_back({{}, max_val_});
  }
  return result;
}

void Scalar::data_save(const hdf5::node::Group& g) const
{
  if (!has_data_)
    return;

  try
  {
    g.attributes.create<double>("value").write(double(data_));
    g.attributes.create<double>("min").write(double(min_val_));
    g.attributes.create<double>("max").write(double(max_val_));
    g.attributes.create<double>("total_count").write(double(total_count_));
  }
  catch (...)
  {
    std::throw_with_nested(std::runtime_error("<Scalar> Could not save"));
  }
}

void Scalar::data_load(const hdf5::node::Group& g)
{
  try
  {
    has_data_ = false;
    if (!g.attributes.exists("value")
        || !g.attributes.exists("min")
        || !g.attributes.exists("max")
        || !g.attributes.exists("total_count"))
      return;

    double val, min, max, tot;
    g.attributes["value"].read(val);
    g.attributes["min"].read(min);
    g.attributes["max"].read(max);
    g.attributes["total_count"].read(tot);

    data_ = val;
    min_val_ = min;
    max_val_ = max;
    total_count_ = tot;
    has_data_ = true;
  }
  catch (...)
  {
    std::throw_with_nested(std::runtime_error("<Scalar> Could not load"));
  }
}

std::string Scalar::data_debug(const std::string& prepend) const
{
  std::stringstream ss;
  if (!has_data_)
    return ss.str();

  ss << prepend
     << "current value = " << data_
     << "   on interval (" << min_val_ << ", " << max_val_ << ")"
     << "   out of total sample = "
     << total_count_
     << "\n";

  return ss.str();
}

void Scalar::export_csv(std::ostream& os) const
{
  if (!has_data_)
    return;

  os << "current value = " << data_
     << "   on interval (" << min_val_ << ", " << max_val_ << ")"
     << "   out of total sample = "
     << total_count_;
}

}
