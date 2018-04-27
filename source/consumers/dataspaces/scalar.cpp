#include "scalar.h"
#include "h5json.h"

namespace DAQuiri
{

Scalar::Scalar()
  : Dataspace(0)
{}

bool Scalar::empty() const
{
  return !has_data_;
}

void Scalar::reserve(const Coords& limits)
{
}

void Scalar::clear()
{
  total_count_ = 0;
  has_data_ = false;
  data_ = 0.0;
}

void Scalar::add(const Entry& e)
{
  if ((e.first.size() != dimensions()) || !e.second)
    return;
  const auto& bin = e.first[0];
  if (bin >= spectrum_.size())
    spectrum_.resize(bin+1, PreciseFloat(0));

  spectrum_[bin] += e.second;
  total_count_ += e.second;
  maxchan_ = std::max(maxchan_, bin);
}
  
void Scalar::add_one(const Coords& coords)
{
  if (coords.size() != dimensions())
    return;
  const auto& bin = coords[0];
  if (bin >= spectrum_.size())
    spectrum_.resize(bin+1, PreciseFloat(0));
  
  spectrum_[bin]++;
  total_count_++;
  maxchan_ = std::max(maxchan_, bin);
}

void Scalar::recalc_axes()
{
  auto ax = axis(0);
  ax.expand_domain(maxchan_);
  set_axis(0, ax);
}

PreciseFloat Scalar::get(const Coords& coords) const
{
  if (coords.size() != dimensions())
    return 0;
  const auto& bin =  *coords.begin();
  if (bin < spectrum_.size())
    return spectrum_.at(bin);
  return 0;
}

EntryList Scalar::range(std::vector<Pair> list) const
{
  size_t min {0};
  size_t max {spectrum_.size() - 1};
  if (list.size() == dimensions())
  {
    min = std::max(list.begin()->first, (long unsigned)(0));
    max = std::min(list.begin()->second, spectrum_.size() - 1);
  }

  EntryList result(new EntryList_t);
  if (spectrum_.empty())
    return result;

  for (size_t i=min; i <= max; ++i)
    result->push_back({{i}, spectrum_[i]});

  return result;
}

void Scalar::data_save(hdf5::node::Group g) const
{
  if (!spectrum_.size())
    return;

  std::vector<double> d(maxchan_ + 1);
  for (uint32_t i = 0; i <= maxchan_; i++)
    d[i] = static_cast<double>(spectrum_[i]);

  auto dtype = hdf5::datatype::create<double>();
  auto dspace = hdf5::dataspace::Simple({d.size()});
  auto dset = g.create_dataset("counts", dtype, dspace);
  dset.write(d);
}

void Scalar::data_load(hdf5::node::Group g)
{
  if (!g.has_dataset("counts"))
    return;

  auto dset = hdf5::node::Dataset(g["counts"]);
  auto shape = hdf5::dataspace::Simple(dset.dataspace()).current_dimensions();
  std::vector<double> rdata(shape[0]);
  dset.read(rdata);

  if (spectrum_.size() < rdata.size())
  {
    spectrum_.clear();
    spectrum_.resize(rdata.size(), PreciseFloat(0));
  }

  maxchan_ = 0;
  total_count_ = 0;
  for (size_t i = 0; i < rdata.size(); i++)
  {
    total_count_ += rdata[i];
    spectrum_[i] = PreciseFloat(rdata[i]);
    if (rdata[i])
      maxchan_ = i;
  }
}

std::string Scalar::data_debug(const std::string &prepend) const
{
  std::stringstream ss;
  if (!spectrum_.size())
    return ss.str();

  uint64_t total  = static_cast<uint64_t>(total_count_);
  uint64_t nstars = static_cast<uint64_t>(maxchan_*3);
  if (!nstars)
    nstars = 100;

  bool print {false};
  for (uint32_t i = 0; i <= maxchan_; i++)
  {
    double val = static_cast<double>(spectrum_[i]);
    if (val)
      print = true;
    if (print)
      ss << prepend << i << ": " <<
            std::string(val*nstars/total,'*') << "\n";
  }

  return ss.str();
}

void Scalar::save(std::ostream& os)
{
  if (!spectrum_.size())
    return;

  for (uint32_t i = 0; i <= maxchan_; i++)
  {
    double val = static_cast<double>(spectrum_[i]);
    os << val << ", ";
  }
}

}
