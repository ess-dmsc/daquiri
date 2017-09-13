#include "dense1d.h"

namespace DAQuiri
{

Dense1D::Dense1D()
  : Dataspace(1)
{}

void Dense1D::reserve(const Coords& limits)
{
  if (limits.size() != dimensions())
    return;
  spectrum_.resize(limits[0], PreciseFloat(0));
}

void Dense1D::clear()
{
  total_count_ = 0;
  spectrum_.clear();
}

void Dense1D::add(const Entry& e)
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
  
void Dense1D::add_one(const Coords& coords)
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

void Dense1D::recalc_axes(uint16_t bits)
{
  auto ax = axis(0);
  if (bits)
    ax.expand_domain(maxchan_, bits);
  else
    ax.expand_domain(maxchan_);
  set_axis(0, ax);
}

PreciseFloat Dense1D::get(const Coords& coords) const
{
  if (coords.size() != dimensions())
    return 0;
  const auto& bin =  *coords.begin();
  if (bin < spectrum_.size())
    return spectrum_.at(bin);
  return 0;
}

EntryList Dense1D::range(std::initializer_list<Pair> list) const
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

void Dense1D::save(H5CC::Group& g) const
{
  std::vector<double> d(maxchan_);
  for (uint32_t i = 0; i <= maxchan_; i++)
    d[i] = static_cast<double>(spectrum_[i]);
  auto dset = g.require_dataset<double>("data", {maxchan_});
  dset.write(d);
}

void Dense1D::load(H5CC::Group& g)
{
  if (!g.has_dataset("data"))
    return;
  H5CC::DataSet dset = g.open_dataset("data");
  H5CC::Shape shape = dset.shape();
  if (shape.rank() != 1)
    return;

  std::vector<double> rdata(shape.dim(0));
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

std::string Dense1D::data_debug(const std::string &prepend) const
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

}
