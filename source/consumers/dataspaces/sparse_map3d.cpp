#include <consumers/dataspaces/sparse_map3d.h>
#include <core/util/ascii_tree.h>
#include <core/util/h5json.h>

#include <core/util/logger.h>

namespace DAQuiri {

SparseMap3D::SparseMap3D()
    : Dataspace(3) {}

bool SparseMap3D::empty() const
{
  return spectrum_.empty();
}

void SparseMap3D::clear()
{
  max0_ = 0;
  max1_ = 0;
  max2_ = 0;
  total_count_ = 0;
  spectrum_.clear();
}

void SparseMap3D::add(const Entry& e)
{
  if ((e.first.size() != dimensions()) || !e.second)
    return;
  bin_pair(e.first[0], e.first[1], e.first[2], e.second);
}

void SparseMap3D::add_one(const Coords& coords)
{
  if (coords.size() != dimensions())
    return;
  bin_one(coords[0], coords[1], coords[2]);
}

void SparseMap3D::recalc_axes()
{
  auto ax0 = axis(0);
  ax0.expand_domain(max0_);
  set_axis(0, ax0);

  auto ax1 = axis(1);
  ax1.expand_domain(max1_);
  set_axis(1, ax1);

  auto ax2 = axis(2);
  ax2.expand_domain(max2_);
  set_axis(2, ax2);
}

PreciseFloat SparseMap3D::get(const Coords& coords) const
{
  if (coords.size() != dimensions())
    return 0;

  tripple point(coords[0], coords[1], coords[2]);

  if (spectrum_.count(point))
    return spectrum_.at(point);
  return 0;
}

EntryList SparseMap3D::range(std::vector<Pair> list) const
{
  size_t min0, min1, min2, max0, max1, max2;
  if (list.size() != dimensions())
  {
    min0 = min1 = min2 = 0;
    max0 = max0_;
    max1 = max1_;
    max2 = max2_;
  }
  else
  {
    const auto& range0 = *list.begin();
    const auto& range1 = *(list.begin() + 1);
    const auto& range2 = *(list.begin() + 2);
    min0 = std::min(range0.first, range0.second);
    max0 = std::max(range0.first, range0.second);
    min1 = std::min(range1.first, range1.second);
    max1 = std::max(range1.first, range1.second);
    min2 = std::min(range2.first, range2.second);
    max2 = std::max(range2.first, range2.second);
  }

  EntryList result(new EntryList_t);
//  Timer makelist(true);

  fill_list(result, min0, max0, min1, max1, min2, max2);

  return result;
}

void SparseMap3D::fill_list(EntryList& result,
                            size_t min0, size_t max0,
                            size_t min1, size_t max1,
                            size_t min2, size_t max2) const
{
  for (const auto& it : spectrum_)
  {
    const auto& co0 = std::get<0>(it.first);
    const auto& co1 = std::get<1>(it.first);
    const auto& co2 = std::get<2>(it.first);
    if ((min0 > co0) || (co0 > max0) ||
        (min1 > co1) || (co1 > max1) ||
        (min2 > co2) || (co2 > max2))
      continue;
    result->push_back({{co0, co1, co2}, it.second});
  }
}

void SparseMap3D::data_save(const hdf5::node::Group& g) const
{
  if (!spectrum_.size())
    return;

  try
  {
    std::vector<uint16_t> dx(spectrum_.size());
    std::vector<uint16_t> dy(spectrum_.size());
    std::vector<uint16_t> dz(spectrum_.size());
    std::vector<double> dc(spectrum_.size());
    size_t i = 0;

    for (auto it = spectrum_.begin(); it != spectrum_.end(); ++it)
    {
      dx[i] = std::get<0>(it->first);
      dy[i] = std::get<1>(it->first);
      dz[i] = std::get<2>(it->first);
      dc[i] = static_cast<long double>(it->second);
      i++;
    }

    using namespace hdf5;

    property::DatasetCreationList dcpl;
    dcpl.layout(property::DatasetLayout::CHUNKED);

    size_t chunksize = spectrum_.size();
    if (chunksize > 128)
      chunksize = 128;

    dataspace::Simple i_space({spectrum_.size(), 3});
    dcpl.chunk({chunksize, 3});
    auto didx = g.create_dataset("indices", datatype::create<uint16_t>(), i_space, dcpl);

    dataspace::Simple c_space({spectrum_.size()});
    dcpl.chunk({chunksize});
    auto dcts = g.create_dataset("counts", datatype::create<double>(), c_space, dcpl);

    dataspace::Hyperslab slab({0, 0}, {static_cast<size_t>(spectrum_.size()), 1});

    slab.offset({0, 0});
    didx.write(dx, slab);

    slab.offset({0, 1});
    didx.write(dy, slab);

    slab.offset({0, 2});
    didx.write(dz, slab);

    dcts.write(dc);
  }
  catch (...)
  {
    std::throw_with_nested(std::runtime_error("<SparseMap3D> Could not save"));
  }
}

void SparseMap3D::data_load(const hdf5::node::Group& g)
{
  try
  {
    using namespace hdf5;

    if (!g.has_dataset("indices") ||
        !g.has_dataset("counts"))
      return;

    auto didx = hdf5::node::Group(g).get_dataset("indices");
    auto dcts = hdf5::node::Group(g).get_dataset("counts");

    auto didx_ds = dataspace::Simple(didx.dataspace()).current_dimensions();
    auto dcts_ds = dataspace::Simple(dcts.dataspace()).current_dimensions();

    dataspace::Hyperslab slab({0, 0}, {static_cast<size_t>(didx_ds[0]), 1});

    std::vector<uint16_t> dx(didx_ds[0], 0);
    slab.offset({0, 0});
    didx.read(dx, slab);

    std::vector<uint16_t> dy(didx_ds[0], 0);
    slab.offset({0, 1});
    didx.read(dy, slab);

    std::vector<uint16_t> dz(didx_ds[0], 0);
    slab.offset({0, 2});
    didx.read(dz, slab);

    std::vector<double> dc(dcts_ds[0], 0.0);
    dcts.read(dc);

    clear();
    for (size_t i = 0; i < dx.size(); ++i)
      bin_pair(dx[i], dy[i], dz[i], dc[i]);
  }
  catch (...)
  {
    std::throw_with_nested(std::runtime_error("<SparseMap3D> Could not load"));
  }
}

std::string SparseMap3D::data_debug(__attribute__((unused)) const std::string& prepend) const
{
  double maximum{0};
  for (auto& b : spectrum_)
    maximum = std::max(maximum, to_double(b.second));

  std::string representation(ASCII_grayscale94);
  std::stringstream ss;

  ss << prepend << "Maximum=" << maximum << "\n";
  if (!maximum)
    return ss.str();

  for (uint16_t i = 0; i <= max0_; i++)
  {
    double total = 0;
    std::stringstream ss2;
    for (uint16_t j = 0; j <= max1_; j++)
    {
      ss2 << prepend << "|";
      for (uint16_t k = 0; k <= max2_; k++)
      {
        uint16_t v = 0;
        if (spectrum_.count(tripple(i, j, k)))
          v = spectrum_.at(tripple(i, j, k)) / maximum * 93;
        total += v;
        ss2 << representation[v];
      }
      ss2 << "\n";
    }
    if (total != 0.0)
    {
      ss << prepend << "x=" << i << "\n";
      ss << ss2.str();
    }
  }

  return ss.str();
}

void SparseMap3D::export_csv(std::ostream& os) const
{
  for (uint16_t i = 0; i <= max0_; i++)
  {
    double total = 0;
    std::stringstream ss2;
    for (uint16_t j = 0; j <= max1_; j++)
    {
      for (uint16_t k = 0; k <= max2_; k++)
      {
        double v = 0;
        if (spectrum_.count(tripple(i, j, k)))
          v = spectrum_.at(tripple(i, j, k));
        total += v;
        ss2 << v;
        if (k!=max2_)
          ss2 << ", ";
      }
      ss2 << ";\n";
    }
    if (total != 0.0)
    {
      os << "x=" << i << "\n";
      os << ss2.str();
    }
  }
}

bool SparseMap3D::is_symmetric()
{
  bool symmetric = true;
//  for (auto &q : spectrum_)
//  {
//    tripple point(q.first.second, q.first.first);
//    if ((!spectrum_.count(point)) || (spectrum_.at(point) != q.second))
//    {
//      symmetric = false;
//      break;
//    }
//  }
  return symmetric;
}

}
