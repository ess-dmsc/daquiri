#include "sparse_matrix2d.h"
#include "ascii_tree.h"
#include "h5json.h"

#include "custom_logger.h"

namespace DAQuiri
{

SparseMatrix2D::SparseMatrix2D()
  : Dataspace(2)
{}

void SparseMatrix2D::reserve(const Coords& limits)
{
  if (limits.size() != dimensions())
    return;
  spectrum_.conservativeResize(limits[0] + 1, limits[1] + 1);
}

void SparseMatrix2D::clear()
{
  total_count_ = 0;
  spectrum_.setZero();
}

void SparseMatrix2D::add(const Entry& e)
{
  if ((e.first.size() != dimensions()) || !e.second)
    return;
  bin_pair(e.first[0], e.first[1], e.second);
}

void SparseMatrix2D::add_one(const Coords& coords)
{
  if (coords.size() != dimensions())
    return;
  bin_one(coords[0], coords[1]);
}

void SparseMatrix2D::recalc_axes()
{
  auto ax0 = axis(0);
  ax0.expand_domain(limits_[0]);
  set_axis(0, ax0);

  auto ax1 = axis(1);
  ax1.expand_domain(limits_[1]);
  set_axis(1, ax1);
}

PreciseFloat SparseMatrix2D::get(const Coords&  coords) const
{
  if (coords.size() != dimensions())
    return 0;  
  return spectrum_.coeff(coords[0], coords[1]);
}

EntryList SparseMatrix2D::range(std::vector<Pair> list) const
{
  int64_t min0, min1, max0, max1;
  if (list.size() != dimensions())
  {
    min0 = min1 = 0;
    max0 = limits_[0];
    max1 = limits_[1];
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

void SparseMatrix2D::fill_list(EntryList& result,
                         int64_t min0, int64_t max0,
                         int64_t min1, int64_t max1) const
{
  for (int k=0; k < spectrum_.outerSize(); ++k)
  {
    for (Eigen::SparseMatrix<double>::InnerIterator it(spectrum_,k); it; ++it)
    {
      
      const auto& co0 = it.row(); // row index
      const auto& co1 = it.col(); // col index (here it is equal to k)
      if ((min0 > co0) || (co0 > max0) ||
          (min1 > co1) || (co1 > max1))
        continue;
      result->push_back({{static_cast<unsigned long>(co0), static_cast<unsigned long>(co1)}, it.value()});
    }
  }
}

#ifdef DAQUIRI_USE_H5
void SparseMatrix2D::save(hdf5::node::Group& g) const
{
  hdf5::error::Singleton::instance().auto_print(false);

  std::vector<uint16_t> dx;
  std::vector<uint16_t> dy;
  std::vector<double> dc;
  for (int k=0; k < spectrum_.outerSize(); ++k)
  {
    for (Eigen::SparseMatrix<double>::InnerIterator it(spectrum_,k); it; ++it)
    {
      dx.push_back(it.row());
      dy.push_back(it.col());
      dc.push_back(static_cast<double>(it.value()));
    }
  }

  DBG << "DC size = " << dc.size();

  using namespace hdf5;
  auto dgroup = g.create_group("data");

  property::LinkCreationList lcpl;
  property::DatasetCreationList dcpl;
  dcpl.layout(property::DatasetLayout::CHUNKED);

  auto i_space = dataspace::Simple({dc.size(),2});
  dcpl.chunk({128,1});
  auto didx = dgroup.create_dataset("indices", datatype::create<uint16_t>(), i_space, lcpl, dcpl);

  auto c_space = dataspace::Simple({dc.size()});
  dcpl.chunk({128});
  auto dcts = dgroup.create_dataset("counts", datatype::create<double>(), c_space);//, lcpl, dcpl);


  dataspace::Hyperslab slab(2);
  slab.block({static_cast<size_t>(dc.size()), 1});

//  slab.offset({0,0});
//  didx.write(dx, slab);
//
//  slab.offset({0,1});
//  didx.write(dy, slab);

//  dcts.write(dc);
}

void SparseMatrix2D::load(hdf5::node::Group& g)
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
  if ((didx_ds.current_dimensions().size() != 2) ||
      (dcts_ds.current_dimensions().size() != 1) ||
      (didx_ds.current_dimensions()[0] !=
          dcts_ds.current_dimensions()[0]))
    return;

  std::vector<uint16_t> dx(didx_ds.current_dimensions()[0]);
  std::vector<uint16_t> dy(didx_ds.current_dimensions()[0]);
  std::vector<double> dc(didx_ds.current_dimensions()[0]);

  hdf5::dataspace::Hyperslab slab;
  slab.block({dx.size(), 1});

  slab.offset(0,0);
  didx.read(dx, slab);

  slab.offset(0,1);
  didx.read(dy, slab);

  dcts.read(dc);

  for (size_t i=0; i < dx.size(); ++i)
    bin_pair(dx[i], dy[i], dc[i]);
}
#endif

std::string SparseMatrix2D::data_debug(__attribute__((unused)) const std::string &prepend) const
{
  double maximum {0};
  for (int k=0; k < spectrum_.outerSize(); ++k)
    for (Eigen::SparseMatrix<double>::InnerIterator it(spectrum_,k); it; ++it)
      maximum = std::max(maximum, to_double(it.value()));
  
  std::string representation(ASCII_grayscale94);
  std::stringstream ss;
  
  ss << "Maximum=" << maximum << "\n";
  if (!maximum)
    return ss.str();
  
  for (uint16_t i = 0; i <= limits_[0]; i++)
  {
    for (uint16_t j = 0; j <= limits_[1]; j++)
    {
      uint16_t v = 0;
      v = spectrum_.coeff(i, j);
      ss << representation[v / maximum * 93];
    }
    ss << "\n";
  }
  
  return ss.str();
}

bool SparseMatrix2D::is_symmetric()
{
  for (int k=0; k < spectrum_.outerSize(); ++k)
    for (Eigen::SparseMatrix<double>::InnerIterator it(spectrum_,k); it; ++it)
      if (spectrum_.coeff(it.col(), it.row()) != it.value())
        return false;
  return true;
}

}
