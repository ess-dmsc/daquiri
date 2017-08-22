#include "dense1d.h"

namespace DAQuiri
{

Dense1D::Dense1D()
  : Dataspace(1)
{}

void Dense1D::_add(const Entry& e)
{
  if ((e.first.size() != _dimensions()) || !e.second)
    return;
  const auto& bin = e.first[0];
  if (bin >= spectrum_.size())
    spectrum_.resize(bin+1, PreciseFloat(0));

  spectrum_[bin] += e.second;
  total_count_ += e.second;
  maxchan_ = std::max(maxchan_, bin);
}

PreciseFloat Dense1D::_get(std::initializer_list<size_t> list) const
{
  if (list.size() != _dimensions())
    return 0;
  const auto& bin =  *list.begin();
  if (bin < spectrum_.size())
    return spectrum_.at(bin);
  return 0;
}

std::unique_ptr<EntryList> Dense1D::_range(std::initializer_list<Pair> list) const
{
  size_t min {0};
  size_t max {spectrum_.size() - 1};
  if (list.size() == _dimensions())
  {
    min = std::max(list.begin()->first, (long unsigned)(0));
    max = std::min(list.begin()->second, spectrum_.size() - 1);
  }

  std::unique_ptr<EntryList> result(new EntryList);
  if (spectrum_.empty())
    return result;

  for (size_t i=min; i <= max; ++i)
    result->push_back({{i}, spectrum_[i]});

  return result;
}

void Dense1D::_save(H5CC::Group& g) const
{
  std::vector<long double> d(maxchan_);
  for (uint32_t i = 0; i <= maxchan_; i++)
    d[i] = static_cast<long double>(spectrum_[i]);
  auto dset = g.require_dataset<long double>("data", {maxchan_});
  dset.write(d);
}

void Dense1D::_load(H5CC::Group& g)
{
  if (!g.has_dataset("data"))
    return;
  H5CC::DataSet dset = g.open_dataset("data");
  H5CC::Shape shape = dset.shape();
  if (shape.rank() != 1)
    return;

  std::vector<long double> rdata(shape.dim(0));
  dset.read(rdata, {rdata.size()}, {0});

  if (spectrum_.size() < rdata.size())
  {
    spectrum_.clear();
    spectrum_.resize(rdata.size(), PreciseFloat(0));
  }

  maxchan_ = 0;
  for (size_t i = 0; i < rdata.size(); i++)
  {
    spectrum_[i] = PreciseFloat(rdata[i]);
    if (rdata[i])
      maxchan_ = i;
  }
}

std::string Dense1D::_data_debug(const std::string &prepend) const
{
  std::stringstream ss;

  uint64_t total  = static_cast<uint64_t>(total_count_);
  uint64_t nstars = static_cast<uint64_t>(maxchan_*3);
  if (!nstars)
    nstars = 100;

  bool print {false};
  for (uint32_t i = 0; i <= maxchan_; i++)
  {
    long double val = static_cast<long double>(spectrum_[i]);
    if (val)
      print = true;
    if (print)
      ss << prepend << i << ": " <<
            std::string(val*nstars/total,'*') << "\n";
  }

  return ss.str();
}

}
