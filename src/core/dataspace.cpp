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


DataAxis Dataspace::axis(uint16_t dimension) const
{
  if (dimension < axes_.size())
    return axes_[dimension];
  else
    return DataAxis();
}

void Dataspace::set_axis(size_t dim, const DataAxis& ax)
{
  if (dim < axes_.size())
    axes_[dim] = ax;
//  else throw?
}

uint16_t Dataspace::dimensions() const
{
  return dimensions_;
}



std::string Dataspace::debug(std::string prepend) const
{
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
     << this->data_debug(prepend + "  ");
  return ss.str();
}

std::string Dataspace::data_debug(const std::string&) const
{
  return std::string();
}

}
