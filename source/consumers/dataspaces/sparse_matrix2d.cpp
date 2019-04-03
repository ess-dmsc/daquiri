#include <consumers/dataspaces/sparse_matrix2d.h>
#include <core/util/ascii_tree.h>
#include <core/util/h5json.h>

#include <core/util/logger.h>

namespace DAQuiri {

SparseMatrix2D::SparseMatrix2D()
    : Dataspace(2) {}

bool SparseMatrix2D::empty() const
{
  return (total_count_ == 0);
}

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
  limits_ = {0,0};
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

PreciseFloat SparseMatrix2D::get(const Coords& coords) const
{
  if (!total_count_ || coords.size() != dimensions())
    return 0;
  if ((coords[0] > limits_[0]) || (coords[1] > limits_[1]))
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
    const auto& range1 = *(list.begin() + 1);
    min0 = std::min(range0.first, range0.second);
    max0 = std::max(range0.first, range0.second);
    min1 = std::min(range1.first, range1.second);
    max1 = std::max(range1.first, range1.second);
  }

  EntryList result(new EntryList_t);
  //  Timer makelist(true);

  fill_list(result, min0, max0, min1, max1);

  return result;
}

void SparseMatrix2D::fill_list(EntryList& result,
                               int64_t min0, int64_t max0,
                               int64_t min1, int64_t max1) const
{
  for (int k = 0; k < spectrum_.outerSize(); ++k)
  {
    for (Eigen::SparseMatrix<double>::InnerIterator it(spectrum_, k); it; ++it)
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

void SparseMatrix2D::data_save(const hdf5::node::Group& g) const
{
  if (spectrum_.nonZeros() == 0)
    return;

  try
  {
    std::vector<uint16_t> dx;
    std::vector<uint16_t> dy;
    std::vector<double> dc;
    for (int k = 0; k < spectrum_.outerSize(); ++k)
    {
      for (Eigen::SparseMatrix<double>::InnerIterator it(spectrum_, k); it; ++it)
      {
        dx.push_back(it.row());
        dy.push_back(it.col());
        dc.push_back(static_cast<double>(it.value()));
      }
    }

    using namespace hdf5;

    property::DatasetCreationList dcpl;
    dcpl.layout(property::DatasetLayout::CHUNKED);

    size_t chunksize = dc.size();
    if (chunksize > 128)
      chunksize = 128;

    auto i_space = dataspace::Simple({dc.size(), 2});
    dcpl.chunk({chunksize, 1});
    auto didx = g.create_dataset("indices", datatype::create<uint16_t>(), i_space, dcpl);

    auto c_space = dataspace::Simple({dc.size()});
    dcpl.chunk({chunksize});
    auto dcts = g.create_dataset("counts", datatype::create<double>(), c_space, dcpl);

    dataspace::Hyperslab slab({0, 0}, {static_cast<size_t>(dc.size()), 1});

    slab.offset({0, 0});
    didx.write(dx, slab);

    slab.offset({0, 1});
    didx.write(dy, slab);

    dcts.write(dc);
  }
  catch (...)
  {
    std::throw_with_nested(std::runtime_error("<SparseMatrix2D> Could not save"));
  }
}

void SparseMatrix2D::data_load(const hdf5::node::Group& g)
{
  try
  {
    using namespace hdf5;

    if (!g.has_dataset("indices") ||
        !g.has_dataset("counts"))
      return;

    auto didx = node::Dataset(g.nodes["indices"]);
    auto dcts = node::Dataset(g.nodes["counts"]);

    auto didx_ds = dataspace::Simple(didx.dataspace()).current_dimensions();
    auto dcts_ds = dataspace::Simple(dcts.dataspace()).current_dimensions();

    dataspace::Hyperslab slab({0, 0}, {static_cast<size_t>(didx_ds[0]), 1});

    std::vector<uint16_t> dx(didx_ds[0], 0);
    slab.offset({0, 0});
    didx.read(dx, slab);

    std::vector<uint16_t> dy(didx_ds[0], 0);
    slab.offset({0, 1});
    didx.read(dy, slab);

    std::vector<double> dc(dcts_ds[0], 0.0);
    dcts.read(dc);

    clear();
    for (size_t i = 0; i < dx.size(); ++i)
      bin_pair(dx[i], dy[i], dc[i]);
  }
  catch (...)
  {
    std::throw_with_nested(std::runtime_error("<SparseMatrix2D> Could not load"));
  }
}

std::string SparseMatrix2D::data_debug(__attribute__((unused)) const std::string& prepend) const
{
  double maximum{0};
  for (int k = 0; k < spectrum_.outerSize(); ++k)
    for (Eigen::SparseMatrix<double>::InnerIterator it(spectrum_, k); it; ++it)
      maximum = std::max(maximum, to_double(it.value()));

  std::string representation(ASCII_grayscale94);
  std::stringstream ss;

  ss << prepend << "Maximum=" << maximum << "\n";
  if (!maximum)
    return ss.str();

  for (uint16_t i = 0; i <= limits_[0]; i++)
  {
    ss << prepend << "|";
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

void SparseMatrix2D::export_csv(std::ostream& os) const
{
  if (!spectrum_.size())
    return;

  for (uint16_t i = 0; i <= limits_[0]; i++)
  {
    for (uint16_t j = 0; j <= limits_[1]; j++)
    {
      os << spectrum_.coeff(i, j);
      if (j < limits_[1])
        os << ", ";
    }
    os << ";\n";
  }
}

bool SparseMatrix2D::is_symmetric()
{
  for (int k = 0; k < spectrum_.outerSize(); ++k)
    for (Eigen::SparseMatrix<double>::InnerIterator it(spectrum_, k); it; ++it)
      if (spectrum_.coeff(it.col(), it.row()) != it.value())
        return false;
  return true;
}

}
