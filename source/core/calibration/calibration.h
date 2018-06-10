#pragma once

#include <boost/date_time.hpp>
#include <core/calibration/coef_function.h>

#include <type_traits>

namespace DAQuiri {

struct CalibID
{
    std::string detector;
    std::string value;
    std::string units;

    CalibID() {}
    CalibID(std::string val, std::string det = "",
            std::string unit = "");

    bool valid() const;
    bool operator== (const CalibID& other) const;
    std::string debug() const;

    friend void to_json(json& j, const CalibID &s);
    friend void from_json(const json& j, CalibID &s);

    bool compare(const CalibID& other);
};

class Calibration
{
  private:
    boost::posix_time::ptime calib_date_
    {boost::posix_time::microsec_clock::universal_time()};
    CalibID from_, to_;
    std::shared_ptr<CoefFunction> function_;

  public:
    Calibration() {}
    Calibration(CalibID from, CalibID to);

    bool valid() const;

    double transform(double) const;
    double inverse_transform(double) const;

    void transform_ref(std::vector<double>&) const;
    std::vector<double> transform_copy(const std::vector<double>&data) const;

    CalibID to() const;
    CalibID from() const;
    std::string model() const;
    boost::posix_time::ptime calib_date() const;
    std::string debug() const;
    std::string fancy_equation(bool with_chi2=false) const;

    void set_function(std::shared_ptr<CoefFunction> f);
    void set_function(const std::string& type, const std::vector<double>& coefs);

    bool shallow_equals(const Calibration& other) const
    {return ((from_ == other.from_) && (to_ == other.to_));}
    bool operator!= (const Calibration& other) const;
    bool operator== (const Calibration& other) const;

    friend void to_json(json& j, const Calibration &s);
    friend void from_json(const json& j, Calibration &s);

    std::string coefs_to_string() const;
    static std::vector<double> coefs_from_string(const std::string&);
};

//use traits!!!

double shift_down(double v, uint16_t bits);
double shift_up(double v, uint16_t bits);
double shift(double v, int16_t bits);

void shift_down(std::vector<double>& vec, uint16_t bits);
void shift_up(std::vector<double>& vec, uint16_t bits);
void shift(std::vector<double>& vec, int16_t bits);


}
