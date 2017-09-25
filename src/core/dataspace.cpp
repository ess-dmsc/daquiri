#include "dataspace.h"
#include "ascii_tree.h"

namespace DAQuiri {

DataAxis::DataAxis(Calibration c, int16_t resample_shift)
{
  calibration = c;
  resample_shift_ = resample_shift;
}

DataAxis::DataAxis(Calibration c, std::vector<double> dom)
{
  calibration = c;
  domain = dom;
}

void DataAxis::expand_domain(size_t ubound)
{
  if (ubound < domain.size())
    return;

  size_t oldbound = domain.size();
  domain.resize(ubound+1);

  for (size_t i=oldbound; i <= ubound; ++i)
  {
    double ii = i;
    if (resample_shift_)
      ii = shift(ii, resample_shift_);
    domain[i] = calibration.transform(ii);
  }
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

EntryList Dataspace::all_data() const
{
  std::vector<Pair> ranges;
  for (auto a : axes_)
    ranges.push_back(a.bounds());
  return this->range(ranges);
}

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
