#include "sparse_map3d.h"
#include "ascii_tree.h"
#include "h5json.h"

namespace DAQuiri
{

SparseMap3D::SparseMap3D()
  : Dataspace(3)
{}

void SparseMap3D::clear()
{
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
    const auto& range1 = *(list.begin()+1);
    const auto& range2 = *(list.begin()+2);
    min0 = std::min(range0.first, range0.second);
    max0 = std::max(range0.first, range0.second);
    min1 = std::min(range1.first, range1.second);
    max1 = std::max(range1.first, range1.second);
    min2 = std::min(range2.first, range2.second);
    max2 = std::max(range2.first, range2.second);
  }

  EntryList result(new EntryList_t);
//  CustomTimer makelist(true);

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

#ifdef DAQUIRI_USE_H5
void SparseMap3D::save(hdf5::node::Group& g) const
{
  auto dgroup = hdf5::require_group(g, "data");

  hdf5::property::LinkCreationList lcpl;
  hdf5::property::DatasetCreationList dcpl;
  dcpl.layout(hdf5::property::DatasetLayout::CHUNKED);

  hdf5::dataspace::Simple i_space({spectrum_.size(),3});
  dcpl.chunk({128,3});
  auto didx = dgroup.create_dataset("indices", hdf5::datatype::create<uint16_t>(), i_space, lcpl, dcpl);

  hdf5::dataspace::Simple c_space({spectrum_.size()});
  dcpl.chunk({128});
  auto dcts = dgroup.create_dataset("counts", hdf5::datatype::create<double>(), c_space, lcpl, dcpl);

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

  hdf5::dataspace::Hyperslab slab;
  slab.block({spectrum_.size(), 1});

  slab.offset({0,0});
  didx.write(dx, slab);

  slab.offset({0,1});
  didx.write(dy, slab);

  slab.offset({0,2});
  didx.write(dz, slab);

  dcts.write(dc);
}

void SparseMap3D::load(hdf5::node::Group& g)
{
  if (!hdf5::has_group(g, "data"))
    return;
  auto dgroup = hdf5::node::Group(g.nodes["data"]);

  if (!hdf5::has_dataset(dgroup, "indices") ||
      !hdf5::has_dataset(dgroup, "counts"))
    return;

  auto didx = hdf5::node::Dataset(dgroup.nodes["indices"]);
  auto dcts = hdf5::node::Dataset(dgroup.nodes["counts"]);

  auto didx_ds = hdf5::dataspace::Simple(didx.dataspace());
  auto dcts_ds = hdf5::dataspace::Simple(dcts.dataspace());
  if ((didx_ds.current_dimensions().size() != 3) ||
      (dcts_ds.current_dimensions().size() != 1) ||
      (didx_ds.current_dimensions()[0] !=
          dcts_ds.current_dimensions()[0]))
    return;

  std::vector<uint16_t> dx(didx_ds.current_dimensions()[0]);
  std::vector<uint16_t> dy(didx_ds.current_dimensions()[0]);
  std::vector<uint16_t> dz(didx_ds.current_dimensions()[0]);
  std::vector<double> dc(didx_ds.current_dimensions()[0]);

  hdf5::dataspace::Hyperslab slab;
  slab.block({dx.size(), 1});

  slab.offset(0,0);
  didx.read(dx, slab);

  slab.offset(0,1);
  didx.read(dy, slab);

  slab.offset(0,2);
  didx.read(dz, slab);

  dcts.read(dc);

  for (size_t i=0; i < dx.size(); ++i)
    bin_pair(dx[i], dy[i], dz[i], dc[i]);
}
#endif

std::string SparseMap3D::data_debug(__attribute__((unused)) const std::string &prepend) const
{
  double maximum {0};
  for (auto &b : spectrum_)
    maximum = std::max(maximum, to_double(b.second));

  std::string representation(ASCII_grayscale94);
  std::stringstream ss;
  std::stringstream ss2;

  ss << "Maximum=" << maximum << "\n";
  if (!maximum)
    return ss.str();

  for (uint16_t i = 0; i <= max0_; i++)
  {
    double total = 0;
    std::stringstream ss2;
    for (uint16_t j = 0; j <= max1_; j++)
    {
      for (uint16_t k = 0; k <= max2_; k++) {
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
      ss << "::: slice=" << i << "\n";
      ss << ss2.str();
      ss << "\n";
    }
  }

  return ss.str();
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
