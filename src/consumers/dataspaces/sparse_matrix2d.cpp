#include "sparse_matrix2d.h"
#include "ascii_tree.h"

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

void SparseMatrix2D::recalc_axes(uint16_t bits)
{
  auto ax0 = axis(0);
  auto ax1 = axis(1);
  if (bits)
  {
    ax0.expand_domain(limits_[0], bits);
    ax1.expand_domain(limits_[1], bits);
  }
  else
  {
    ax0.expand_domain(limits_[0]);
    ax1.expand_domain(limits_[1]);
  }
  set_axis(0, ax0);
  set_axis(1, ax1);
}

PreciseFloat SparseMatrix2D::get(const Coords&  coords) const
{
  if (coords.size() != dimensions())
    return 0;  
  return spectrum_.coeff(coords[0], coords[1]);
}

EntryList SparseMatrix2D::range(std::initializer_list<Pair> list) const
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

void SparseMatrix2D::fill_list(EntryList& result,
                         size_t min0, size_t max0,
                         size_t min1, size_t max1) const
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

void SparseMatrix2D::save(H5CC::Group& g) const
{
  auto dgroup = g.require_group("data");
  auto didx = dgroup.require_dataset<uint16_t>("indices", {static_cast<unsigned long long>(spectrum_.nonZeros()), 2}, {128,2});
  auto dcts = dgroup.require_dataset<double>("counts", {static_cast<unsigned long long>(spectrum_.nonZeros())}, {128});
  std::vector<uint16_t> dx(spectrum_.size());
  std::vector<uint16_t> dy(spectrum_.size());
  std::vector<double> dc(spectrum_.size());
  size_t i = 0;
  for (int k=0; k < spectrum_.outerSize(); ++k) {
    for (Eigen::SparseMatrix<double>::InnerIterator it(spectrum_,k); it; ++it) {
      dx[i] = it.row();
      dy[i] = it.col();
      dc[i] = static_cast<double>(it.value());
      i++;
    }
  }
  didx.write(dx, {static_cast<unsigned long long>(spectrum_.nonZeros()), 1}, {0,0});
  didx.write(dy, {static_cast<unsigned long long>(spectrum_.nonZeros()), 1}, {0,1});
  dcts.write(dc);
}

void SparseMatrix2D::load(H5CC::Group& g)
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
  std::vector<double> dc(didx.shape().dim(0));
  
  didx.read(dx, {dx.size(), 1}, {0,0});
  didx.read(dy, {dy.size(), 1}, {0,1});
  dcts.read(dc, {dx.size()}, {0});
  
  for (size_t i=0; i < dx.size(); ++i)
    bin_pair(dx[i], dy[i], dc[i]);
}

std::string SparseMatrix2D::data_debug(const std::string &prepend) const
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
