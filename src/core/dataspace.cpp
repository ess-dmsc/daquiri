#include "dataspace.h"
#include "ascii_tree.h"

namespace DAQuiri {

DataAxis::DataAxis(Calibration c, size_t resolution)
{
  calibration = c;
  domain.resize(resolution);
  for (size_t i=0; i < resolution; ++i)
    domain[i] = calibration.transform(i);
}

DataAxis::DataAxis(Calibration c, size_t resolution, uint16_t bits)
{
  calibration = c;
  domain.resize(resolution);
  for (size_t i=0; i < resolution; ++i)
    domain[i] = calibration.transform(i, bits);
}

void DataAxis::expand_domain(size_t ubound)
{
  if (ubound < domain.size())
    return;

  size_t oldbound = domain.size();
  domain.resize(ubound+1);

  for (size_t i=oldbound; i <= ubound; ++i)
    domain[i] = calibration.transform(i);
}

void DataAxis::expand_domain(size_t ubound, uint16_t bits)
{
  if (ubound < domain.size())
    return;

  size_t oldbound = domain.size();
  domain.resize(ubound+1);

  for (size_t i=oldbound; i <= ubound; ++i)
    domain[i] = calibration.transform(i, bits);
}

Pair DataAxis::bounds() const
{
  return Pair(0, domain.size() - 1);
}

std::string DataAxis::label() const
{
//  if (!calibration.valid())
//    return "undefined axis";
  std::stringstream ss;
  if (!calibration.to().value.empty())
    ss << calibration.to().value;
  else
    ss << calibration.from().value;
  if (!calibration.to().units.empty())
    ss << " (" << calibration.to().units << ")";
  else
    ss << " (" << calibration.from().bits << " bits)";
  return ss.str();
}

std::string DataAxis::debug() const
{
  std::stringstream ss;
  ss << "domain_size=" << domain.size();
  if (domain.size())
    ss << " [" << domain[0]
       << "-" << domain[domain.size()-1]
       << "]";
  if (calibration.valid())
    ss << " " << calibration.debug();
  return ss.str();
}


Dataspace::Dataspace()
{}

Dataspace::Dataspace(uint16_t dimensions)
  : dimensions_(dimensions)
{
  axes_.resize(dimensions);
}

Dataspace::Dataspace(const Dataspace& other)
  : dimensions_(other.dimensions_)
  , axes_ (other.axes_)
{}

PreciseFloat Dataspace::get(std::initializer_list<size_t> list) const
{
  SHARED_LOCK_ST
  return this->_get(list);
}

EntryList Dataspace::range(std::initializer_list<Pair> list) const
{
  SHARED_LOCK_ST
  return this->_range(list);
}

void Dataspace::recalc_axes(uint16_t bits)
{
  UNIQUE_LOCK_EVENTUALLY_ST
  this->_recalc_axes(bits);
}

void Dataspace::clear()
{
  UNIQUE_LOCK_EVENTUALLY_ST
  this->_clear();
}

void Dataspace::add(const Entry& e)
{
  UNIQUE_LOCK_EVENTUALLY_ST
  this->_add(e);
}

DataAxis Dataspace::axis(uint16_t dimension) const
{
  SHARED_LOCK_ST
  return _axis(dimension);
}

DataAxis Dataspace::_axis(uint16_t dimension) const
{
  if (dimension < axes_.size())
    return axes_[dimension];
  else
    return DataAxis();
}

void Dataspace::set_axis(size_t dim, const DataAxis& ax)
{
  UNIQUE_LOCK_EVENTUALLY_ST
  _set_axis(dim, ax);
}

void Dataspace::_set_axis(size_t dim, const DataAxis& ax)
{
  if (dim < axes_.size())
    axes_[dim] = ax;
//  else throw?
}

uint16_t Dataspace::dimensions() const
{
  SHARED_LOCK_ST
  return _dimensions();
}

uint16_t Dataspace::_dimensions() const
{
  return dimensions_;
}

std::string Dataspace::debug(std::string prepend) const
{
  SHARED_LOCK_ST
  std::stringstream ss;
  if (axes_.empty())
    ss << prepend << k_branch_mid_B << "Axes undefined\n";
  else
  {
    ss << prepend << k_branch_mid_B << "Axes:\n";
    for (size_t i=0; i < axes_.size();++i)
    {
      ss << prepend << k_branch_pre_B
         << (((i+1) == axes_.size()) ? k_branch_end_B : k_branch_mid_B)
         << i << "   " << axes_.at(i).debug()
         << "\n";
    }
  }
  ss << prepend << k_branch_end_B << "Data:\n"
     << this->_data_debug(prepend + "  ");
  return ss.str();
}

void Dataspace::load(H5CC::Group& g)
{
  UNIQUE_LOCK_EVENTUALLY_ST
  this->_load(g);
}

void Dataspace::save(H5CC::Group& g) const
{
  SHARED_LOCK_ST
  this->_save(g);
}

std::string Dataspace::_data_debug(const std::string&) const
{
  return std::string();
}

}
