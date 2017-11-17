#include "dense_matrix2d.h"
#include "ascii_tree.h"
#include "h5json.h"

namespace DAQuiri
{

DenseMatrix2D::DenseMatrix2D()
  : Dataspace(2)
  , spectrum_(100,100)
{}

void DenseMatrix2D::reserve(const Coords& limits)
{
  if (limits.size() != dimensions())
    return;
  spectrum_.conservativeResize(limits[0] + 1, limits[1] + 1);
}

void DenseMatrix2D::clear()
{
  total_count_ = 0;
  spectrum_.setZero();
}

void DenseMatrix2D::add(const Entry& e)
{
  if ((e.first.size() != dimensions()) || !e.second)
    return;
  bin_pair(e.first[0], e.first[1], e.second);
}

void DenseMatrix2D::add_one(const Coords& coords)
{
  if (coords.size() != dimensions())
    return;
  bin_one(coords[0], coords[1]);
}

void DenseMatrix2D::recalc_axes()
{
  auto ax0 = axis(0);
  ax0.expand_domain(limits_[0]);
  set_axis(0, ax0);

  auto ax1 = axis(1);
  ax1.expand_domain(limits_[1]);
  set_axis(1, ax1);
}

PreciseFloat DenseMatrix2D::get(const Coords&  coords) const
{
  if (coords.size() != dimensions())
    return 0;  
  return spectrum_.coeff(coords[0], coords[1]);
}

EntryList DenseMatrix2D::range(std::vector<Pair> list) const
{
  size_t min0, min1, max0, max1;
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

void DenseMatrix2D::fill_list(EntryList& result,
                         size_t min0, size_t max0,
                         size_t min1, size_t max1) const
{
  for (size_t k=min0; k <= max0; ++k)
  {
    for (size_t l=min1; l <= max1; ++l)
    {
      result->push_back({{static_cast<unsigned long>(k),
                          static_cast<unsigned long>(l)},
                          spectrum_.coeff(k,l)});
    }
  }
}

void DenseMatrix2D::save(hdf5::node::Group& g) const
{
  auto dgroup = hdf5::require_group(g, "data");

  hdf5::property::LinkCreationList lcpl;
  hdf5::property::DatasetCreationList dcpl;
  dcpl.layout(hdf5::property::DatasetLayout::CHUNKED);

  hdf5::dataspace::Simple i_space({static_cast<unsigned long long>(spectrum_.nonZeros()),2});
  dcpl.chunk({128,2});
  auto didx = dgroup.create_dataset("indices", hdf5::datatype::create<uint16_t>(), i_space, lcpl, dcpl);

  hdf5::dataspace::Simple c_space({static_cast<unsigned long long>(spectrum_.nonZeros())});
  dcpl.chunk({128});
  auto dcts = dgroup.create_dataset("counts", hdf5::datatype::create<double>(), c_space, lcpl, dcpl);

  std::vector<uint16_t> dx(spectrum_.size());
  std::vector<uint16_t> dy(spectrum_.size());
  std::vector<double> dc(spectrum_.size());
//  size_t i = 0;
//  for (int k=0; k < spectrum_.outerSize(); ++k) {
//    for (data_type_t::InnerIterator it(spectrum_,k); it; ++it) {
//      dx[i] = it.row();
//      dy[i] = it.col();
//      dc[i] = static_cast<double>(it.value());
//      i++;
//    }
//  }
  hdf5::dataspace::Hyperslab slab;
  slab.block({static_cast<unsigned long long>(spectrum_.nonZeros()), 1});

  slab.offset({0,0});
  didx.write(dx, slab);

  slab.offset({0,1});
  didx.write(dy, slab);

  dcts.write(dc);
}

void DenseMatrix2D::load(hdf5::node::Group& g)
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

std::string DenseMatrix2D::data_debug(const std::string &prepend) const
{
  uint64_t maximum {0};
  for (int k=0; k < spectrum_.rows(); ++k)
    for (int l=0; l < spectrum_.cols(); ++l)
      maximum = std::max(maximum, spectrum_(k,l));
  
  std::string representation(ASCII_grayscale94);
  std::stringstream ss;
  
  ss << "Maximum=" << maximum
     << " rows=" << spectrum_.rows()
     << " cols=" << spectrum_.cols()
     << "\n";
  if (!maximum)
    return ss.str();
  
  for (uint16_t i = 0; i <= limits_[0]; i++)
  {
    for (uint16_t j = 0; j <= limits_[1]; j++)
    {
//      uint16_t v = 0;
      double v = spectrum_(i, j);
      ss << v << " ";

//      ss << representation[static_cast<uint16_t>(v / maximum * 93)];
    }
    ss << "\n";
  }
  
  return ss.str();
}

bool DenseMatrix2D::is_symmetric()
{
  for (int k=0; k < spectrum_.rows(); ++k)
    for (int l=0; l < spectrum_.cols(); ++l)
      if (spectrum_.coeff(k, l) != spectrum_.coeff(l, k))
        return false;
  return true;
}

}
