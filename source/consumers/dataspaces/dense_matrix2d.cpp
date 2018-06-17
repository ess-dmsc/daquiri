#include <consumers/dataspaces/dense_matrix2d.h>
#include <core/util/ascii_tree.h>
#include <core/util/h5json.h>

namespace DAQuiri
{

DenseMatrix2D::DenseMatrix2D()
  : Dataspace(2)
  , spectrum_(100,100)
{}

bool DenseMatrix2D::empty() const
{
  return (total_count_ == 0);
}

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

void DenseMatrix2D::data_save(const hdf5::node::Group& g) const
{
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

  using namespace hdf5;

  property::DatasetCreationList dcpl;
  dcpl.layout(property::DatasetLayout::CHUNKED);

  size_t chunksize = dc.size();
  if (chunksize > 128)
    chunksize = 128;

  auto i_space = dataspace::Simple({static_cast<size_t>(spectrum_.nonZeros()), 2});
  dcpl.chunk({chunksize, 2});
  auto didx = g.create_dataset("indices", datatype::create<uint16_t>(), i_space, dcpl);

  dataspace::Simple c_space({static_cast<size_t>(spectrum_.nonZeros())});
  dcpl.chunk({chunksize});
  auto dcts = g.create_dataset("counts", datatype::create<double>(), c_space, dcpl);

  dataspace::Hyperslab slab(2);
  slab.block({static_cast<size_t>(spectrum_.nonZeros()), 1});

  slab.offset({0, 0});
  didx.write(dx, slab);

  slab.offset({0, 1});
  didx.write(dy, slab);

  dcts.write(dc);
}

void DenseMatrix2D::data_load(const hdf5::node::Group& g)
{
  if (!g.has_dataset("indices") ||
      !g.has_dataset("counts"))
    return;

  using namespace hdf5;

  auto didx = hdf5::node::Group(g).get_dataset("indices");
  auto dcts = hdf5::node::Group(g).get_dataset("counts");

  auto didx_ds = dataspace::Simple(didx.dataspace()).current_dimensions();
  auto dcts_ds = dataspace::Simple(dcts.dataspace()).current_dimensions();

  dataspace::Hyperslab slab({0, 0}, {static_cast<size_t>(didx_ds[0]), 1});

  std::vector<uint16_t> dx(didx_ds[0]);
  slab.offset({0, 0});
  didx.read(dx, slab);

  std::vector<uint16_t> dy(didx_ds[0]);
  slab.offset({0, 1});
  didx.read(dy, slab);

  std::vector<double> dc(didx_ds[0]);
  dcts.read(dc);

  for (size_t i = 0; i < dx.size(); ++i)
    bin_pair(dx[i], dy[i], dc[i]);
}

std::string DenseMatrix2D::data_debug(__attribute__((unused)) const std::string &prepend) const
{
  uint64_t maximum {0};
  for (int k=0; k < spectrum_.rows(); ++k)
    for (int l=0; l < spectrum_.cols(); ++l)
      maximum = std::max(maximum, spectrum_(k,l));
  
  std::string representation(ASCII_grayscale94);
  std::stringstream ss;
  
  ss << prepend << "Maximum=" << maximum
     << " rows=" << spectrum_.rows()
     << " cols=" << spectrum_.cols()
     << "\n";
  if (!maximum)
    return ss.str();
  
  for (uint16_t i = 0; i <= limits_[0]; i++)
  {
    ss << prepend << "|";
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
