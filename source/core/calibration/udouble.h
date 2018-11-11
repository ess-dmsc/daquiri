// Adapted from "Uncertainty Propagation in C++" by Evan Manning, 1996

#pragma once

#include <iostream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

//  model  uncertain  number  using  only  mean  and  sigma  (pure  Gaussian)
template<bool is_correlated>
class UDouble
{
 private:
  double value;
  double uncertainty;
 public:
  UDouble(const double val = 0.0, const double unc = 0.0);
  UDouble(const UDouble& ud);
  ~UDouble() = default;

  UDouble<is_correlated> operator+() const;
  UDouble<is_correlated> operator-() const;

  friend UDouble<is_correlated> operator+(UDouble<is_correlated>,
                                          const UDouble<is_correlated>&);
  friend UDouble<is_correlated> operator-(UDouble<is_correlated>,
                                          const UDouble<is_correlated>&);
  UDouble<is_correlated> operator++();
  UDouble<is_correlated> operator--();
  UDouble<is_correlated> operator++(int);
  UDouble<is_correlated> operator--(int);
  friend UDouble<is_correlated> operator*(UDouble<is_correlated>,
                                          const UDouble<is_correlated>&);

  friend UDouble<is_correlated> operator/(UDouble<is_correlated>,
                                          const UDouble<is_correlated>&);

  UDouble<is_correlated>& operator+=(const UDouble<is_correlated>&);
  UDouble<is_correlated>& operator-=(const UDouble<is_correlated>&);
  UDouble<is_correlated>& operator*=(const UDouble<is_correlated>&);
  UDouble<is_correlated>& operator/=(const UDouble<is_correlated>&);
  friend std::ostream& operator<<(std::ostream&, const UDouble<is_correlated>&);
  friend std::istream& operator>(std::istream&, UDouble<is_correlated>);
  //  math  library  functions
  friend UDouble<is_correlated> ceil(UDouble<is_correlated>);
  friend UDouble<is_correlated> floor(UDouble<is_correlated>);
  friend UDouble<is_correlated> fabs(UDouble<is_correlated>);
  friend UDouble<is_correlated> ldexp(UDouble<is_correlated>, int);
  friend UDouble<is_correlated> modf(UDouble<is_correlated>, double*);
  friend UDouble<is_correlated> frexp(UDouble<is_correlated>, int*);
  friend UDouble<is_correlated> fmod(const UDouble<is_correlated>&,
                                     const UDouble<is_correlated>&);
  friend UDouble<is_correlated> sqrt(UDouble<is_correlated>);
  friend UDouble<is_correlated> sin(UDouble<is_correlated>);
  friend UDouble<is_correlated> cos(UDouble<is_correlated>);
  friend UDouble<is_correlated> tan(UDouble<is_correlated>);
  friend UDouble<is_correlated> asin(UDouble<is_correlated>);
  friend UDouble<is_correlated> acos(UDouble<is_correlated>);
  friend UDouble<is_correlated> atan(UDouble<is_correlated>);
  friend UDouble<is_correlated> atan2(const UDouble<is_correlated>&,
                                      const UDouble<is_correlated>&);
  friend UDouble<is_correlated> exp(UDouble<is_correlated>);
  friend UDouble<is_correlated> log(UDouble<is_correlated>);
  friend UDouble<is_correlated> log10(UDouble<is_correlated>);
  friend UDouble<is_correlated> sinh(UDouble<is_correlated>);
  friend UDouble<is_correlated> cosh(UDouble<is_correlated>);
  friend UDouble<is_correlated> tanh(UDouble<is_correlated>);
  friend UDouble<is_correlated> pow(const UDouble<is_correlated>&,
                                    const UDouble<is_correlated>&);
  //  read-only  access  to  data  members
  double mean() const;
  double deviation() const;
  friend UDouble<is_correlated> PropagateUncertaintiesBySlope(
      double  (*)(double),
      const UDouble<is_correlated>&);
  friend UDouble<is_correlated> PropagateUncertaintiesBySlope(
      double  (*)(double, double),
      const UDouble<is_correlated>&,
      const UDouble<is_correlated>&);
};

#include <core/calibration/udouble.tpp>

using UDoubleCorr = UDouble<true>;
using UDoubleUncorr = UDouble<false>;
