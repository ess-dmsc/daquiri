// Adapted from "Uncertainty Propagation in C++" by Evan Manning, 1996

#pragma once

#include <cmath>

//  prints  uncertainty  to  2  digits  and  value  to  same  precision
inline void uncertain_print(double mean, double sigma,
                            std::ostream& os = std::cout)
{
  auto original_precision = os.precision();
  auto original_format = os.flags(std::ios::showpoint);
  int precision;
  //  special  cases  for  zero,  NaN,  and  Infinities
  //  (positive  &  negative)
  if ((sigma == 0.0) || (sigma != sigma) || (1.0 / sigma == 0.0))
  {
    precision = 0;
  }
  else
  {
    //  round  sigma  to  2  digits
    int sigma_digits = 1 - int(floor(log10(fabs(sigma))));
    double round_10_pow = pow(10.0, sigma_digits);
    sigma = floor(sigma * round_10_pow + 0.5) / round_10_pow;
    //  round  mean  to  same  #  of  digits
    mean = floor(mean * round_10_pow + 0.5) / round_10_pow;
    if (mean == 0.0)
    {
      if (sigma_digits > 0)
        precision = sigma_digits + 1;
      else
        precision = 1;
    }
    else
    {
      precision = int(floor(log10(fabs(mean)))) + sigma_digits + 1;
      if (precision < 1)
      {
        mean = 0.0;
        if (sigma_digits > 0)
          precision = sigma_digits + 1;
        else
          precision = 1;
      }
    }
  }
  os << std::setprecision(precision)
     << mean << "  +/-  "
     << std::setprecision(2)
     << sigma
     << std::setprecision(original_precision);
  os.flags(original_format);
}

template<bool is_correlated>
UDouble<is_correlated>::UDouble(double val, double unc)
    : value(val), uncertainty(unc)
{
  if ((unc < 0.0) && !is_correlated)
  {
    std::stringstream ss;
    ss << "Error:  negative  uncertainty: " << unc;
    throw std::runtime_error(ss.str());
  }
}

template<bool is_correlated>
UDouble<is_correlated>::UDouble(const UDouble& ud)
    : value(ud.value), uncertainty(ud.uncertainty) {}

template<bool is_correlated>
UDouble<is_correlated> UDouble<is_correlated>::operator+() const { return *this; }

template<bool is_correlated>
UDouble<is_correlated> UDouble<is_correlated>::operator-() const
{
  if (is_correlated)
    return UDouble<is_correlated>(-value, -uncertainty);
  else
    return UDouble<is_correlated>(-value, uncertainty);
}

template<bool is_correlated>
double UDouble<is_correlated>::mean() const {
  return value;
}

template<bool is_correlated>
double UDouble<is_correlated>::deviation() const {
  return uncertainty;
}

template<bool is_correlated>
UDouble<is_correlated> sin(UDouble<is_correlated> arg)
{
  if (is_correlated)
    arg.uncertainty *= cos(arg.value);
  else
    arg.uncertainty *= fabs(cos(arg.value));
  arg.value = sin(arg.value);
  return arg;
}

template<bool is_correlated>
UDouble<is_correlated> exp(UDouble<is_correlated> arg)
{
  arg.value = exp(arg.value);
  if (is_correlated)
    arg.uncertainty *= arg.value;
  else
    arg.uncertainty *= fabs(arg.value);
  return arg;
}

template<bool is_correlated>
UDouble<is_correlated> ceil(UDouble<is_correlated> arg)
{
  arg.value = ceil(arg.value);
  arg.uncertainty = 0.0;
  return arg;
}

template<bool is_correlated>
UDouble<is_correlated> PropagateUncertaintiesBySlope(
    double  (* certain_func)(double),
    const UDouble<is_correlated>& arg)
{
  UDouble<is_correlated> retval;
  double sigma_up_value, sigma_down_value;
  retval.value = certain_func(arg.value);
  sigma_up_value = certain_func(arg.value + arg.uncertainty);
  sigma_down_value = certain_func(arg.value - arg.uncertainty);
  retval.uncertainty = (sigma_up_value - sigma_down_value) * 0.5;
  if (!is_correlated)
    retval.uncertainty = fabs(retval.uncertainty);
  return retval;
}

template<bool is_correlated>
UDouble<is_correlated> PropagateUncertaintiesBySlope(
    double  (* certain_func)(double, double),
    const UDouble<is_correlated>& arg1,
    const UDouble<is_correlated>& arg2)
{
  UDouble<is_correlated> retval;
  retval.value = certain_func(arg1.value, arg2.value);
  if (is_correlated)
  {
    double up_val = certain_func(arg1.value + arg1.uncertainty,
                                 arg2.value + arg2.uncertainty);
    double down_val = certain_func(arg1.value - arg1.uncertainty,
                                   arg2.value - arg2.uncertainty);
    retval.uncertainty = 0.5 * (up_val - down_val);
  }
  else
  {
    double up_val1 = certain_func(arg1.value + arg1.uncertainty,
                                  arg2.value);
    double down_val1 = certain_func(arg1.value - arg1.uncertainty,
                                    arg2.value);
    double up_val2 = certain_func(arg1.value,
                                  arg2.value + arg2.uncertainty);
    double down_val2 = certain_func(arg1.value,
                                    arg2.value - arg2.uncertainty);
    retval.uncertainty = 0.5 * hypot(up_val1 - down_val1,
                                     up_val2 - down_val2);
  }
  return retval;
}

template<bool is_correlated>
UDouble<is_correlated>& UDouble<is_correlated>::operator+=(const UDouble<is_correlated>& ud)
{
  if (is_correlated)
    uncertainty += ud.uncertainty;
  else
    uncertainty = hypot(uncertainty, ud.uncertainty);
  value += ud.value;
  return *this;
}

template<bool is_correlated>
UDouble<is_correlated>& UDouble<is_correlated>::operator/=(const UDouble<is_correlated>& ud)
{
  if (is_correlated)
    uncertainty = uncertainty / ud.value
        - (ud.uncertainty * value) / (ud.value * ud.value);
  else
    uncertainty =
        hypot(uncertainty / ud.value,
              (ud.uncertainty * value) / (ud.value * ud.value));
  value /= ud.value;
  return *this;
}

template<bool is_correlated>
UDouble<is_correlated> atan2(const UDouble<is_correlated>& arg1,
                             const UDouble<is_correlated>& arg2)
{
  UDouble<is_correlated> retval;
  double slope1 = 1.0, slope2 = 1.0;
  double sum2 = arg2.value * arg2.value + arg1.value * arg1.value;

  if (sum2 != 0.0)
  {
    slope1 = arg2.value / sum2;
    slope2 = -arg1.value / sum2;
  }
  if (is_correlated)
    retval.uncertainty = slope1 * arg1.uncertainty
        + slope2 * arg2.uncertainty;
  else
    retval.uncertainty = hypot(slope1 * arg1.uncertainty,
                               slope2 * arg2.uncertainty);
  retval.value = atan2(arg1.value, arg2.value);
  return retval;
}
