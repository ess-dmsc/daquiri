#include "sparse_map2d.h"
#include "ascii_tree.h"

namespace DAQuiri
{

SparseMap2D::SparseMap2D()
  : Dataspace(2)
{}

void SparseMap2D::clear()
{
  total_count_ = 0;
  spectrum_.clear();
}

void SparseMap2D::add(const Entry& e)
{
  if ((e.first.size() != dimensions()) || !e.second)
    return;
  bin_pair(e.first[0], e.first[1], e.second);
}

void SparseMap2D::add_one(const Coords& coords)
{
  if (coords.size() != dimensions())
    return;
  bin_one(coords[0], coords[1]);
}

void SparseMap2D::recalc_axes(uint16_t bits)
{
  auto ax0 = axis(0);
  auto ax1 = axis(1);
  if (bits)
  {
    ax0.expand_domain(max0_, bits);
    ax1.expand_domain(max1_, bits);
  }
  else
  {
    ax0.expand_domain(max0_);
    ax1.expand_domain(max1_);
  }
  set_axis(0, ax0);
  set_axis(1, ax1);
}

PreciseFloat SparseMap2D::get(const Coords& coords) const
{
  if (coords.size() != dimensions())
    return 0;

  std::pair<uint16_t,uint16_t> point(coords[0], coords[1]);

  if (spectrum_.count(point))
    return spectrum_.at(point);
  return 0;
}

EntryList SparseMap2D::range(std::initializer_list<Pair> list) const
{
  size_t min0, min1, max0, max1;
  if (list.size() != dimensions())
  {
    min0 = min1 = 0;
    max0 = max0_;
    max1 = max1_;
  }
  else
  {
    const auto& range0 = *list.begin();
    const auto& range1 = *(list.begin()+1);
    min0 = std::min(range0.first, range0.second);
    max0 = std::max(range0.first, range0.second);
    min1 = std::min(range1.first, range1.second);
    max1 = std::max(range1.first, range1.second);
  }

  EntryList result(new EntryList_t);
//  CustomTimer makelist(true);

  fill_list(result, min0, max0, min1, max1);

  return result;
}

void SparseMap2D::fill_list(EntryList& result,
                          size_t min0, size_t max0,
                          size_t min1, size_t max1) const
{
  for (const auto& it : spectrum_)
  {
    const auto& co0 = it.first.first;
    const auto& co1 = it.first.second;
    if ((min0 > co0) || (co0 > max0) ||
        (min1 > co1) || (co1 > max1))
      continue;
    result->push_back({{co0, co1}, it.second});
  }
}

void SparseMap2D::save(H5CC::Group& g) const
{
  auto dgroup = g.require_group("data");
  auto didx = dgroup.require_dataset<uint16_t>("indices", {spectrum_.size(), 2}, {128,2});
  auto dcts = dgroup.require_dataset<long double>("counts", {spectrum_.size()}, {128});
  std::vector<uint16_t> dx(spectrum_.size());
  std::vector<uint16_t> dy(spectrum_.size());
  std::vector<long double> dc(spectrum_.size());
  size_t i = 0;
  for (auto it = spectrum_.begin(); it != spectrum_.end(); ++it)
  {
    dx[i] = it->first.first;
    dy[i] = it->first.second;
    dc[i] = static_cast<long double>(it->second);
    i++;
  }
  didx.write(dx, {spectrum_.size(), 1}, {0,0});
  didx.write(dy, {spectrum_.size(), 1}, {0,1});
  dcts.write(dc);
}

void SparseMap2D::load(H5CC::Group& g)
{
  if (!g.has_group("data"))
    return;
  auto dgroup = g.open_group("data");

  if (!dgroup.has_dataset("indices") || !dgroup.has_dataset("counts"))
    return;

  auto didx = dgroup.open_dataset("indices");
  auto dcts = dgroup.open_dataset("counts");

  if ((didx.shape().rank() != 2) ||
      (dcts.shape().rank() != 1) ||
      (didx.shape().dim(0) != dcts.shape().dim(0)))
    return;

  std::vector<uint16_t> dx(didx.shape().dim(0));
  std::vector<uint16_t> dy(didx.shape().dim(0));
  std::vector<long double> dc(didx.shape().dim(0));

  didx.read(dx, {dx.size(), 1}, {0,0});
  didx.read(dy, {dy.size(), 1}, {0,1});
  dcts.read(dc, {dx.size()}, {0});

  for (size_t i=0; i < dx.size(); ++i)
    bin_pair(dx[i], dy[i], dc[i]);
}

std::string SparseMap2D::data_debug(const std::string &prepend) const
{
  double maximum {0};
  for (auto &b : spectrum_)
    maximum = std::max(maximum, to_double(b.second));

  std::string representation(ASCII_grayscale94);
  std::stringstream ss;

  ss << "Maximum=" << maximum << "\n";
  if (!maximum)
    return ss.str();

  for (uint16_t i = 0; i <= max0_; i++)
  {
    for (uint16_t j = 0; j <= max1_; j++)
    {
      uint16_t v = 0;
      if (spectrum_.count(std::pair<uint16_t, uint16_t>(i,j)))
        v = spectrum_.at(std::pair<uint16_t, uint16_t>(i,j));
      ss << representation[v / maximum * 93];
    }
    ss << "\n";
  }

  return ss.str();
}

bool SparseMap2D::is_symmetric()
{
  bool symmetric = true;
  for (auto &q : spectrum_)
  {
    std::pair<uint16_t,uint16_t> point(q.first.second, q.first.first);
    if ((!spectrum_.count(point)) || (spectrum_.at(point) != q.second))
    {
      symmetric = false;
      break;
    }
  }
  return symmetric;
}

}
