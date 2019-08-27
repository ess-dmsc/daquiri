#include <core/dataspace.h>
#include <core/util/ascii_tree.h>
#include <core/util/h5json.h>

#include <codecvt>
#include <locale>

namespace DAQuiri {

DataAxis::DataAxis(Calibration c, int16_t resample_shift) {
  calibration = c;
  resample_shift_ = resample_shift;
}

DataAxis::DataAxis(Calibration c, std::vector<double> dom) {
  calibration = c;
  domain = dom;
}

void DataAxis::expand_domain(size_t ubound) {
  if (ubound < domain.size())
    return;

  size_t oldbound = domain.size();
  domain.resize(ubound + 1);

  for (size_t i = oldbound; i <= ubound; ++i) {
    double ii = i;
    if (resample_shift_)
      ii = shift(ii, resample_shift_);
    domain[i] = calibration.transform(ii);
  }
}

Pair DataAxis::bounds() const { return Pair(0, domain.size() - 1); }

std::string DataAxis::label() const {
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

std::string DataAxis::debug() const {
  std::stringstream ss;
  ss << "domain_size=" << domain.size();
  if (domain.size())
    ss << " [" << domain[0] << "-" << domain[domain.size() - 1] << "]";
  if (resample_shift_)
    ss << " [resample_shift=" << resample_shift_ << "]";
  ss << " " << calibration.debug();
  return ss.str();
}

void to_json(json &j, const DataAxis &da) {
  j["calibration"] = da.calibration;
  j["resample_shift"] = da.resample_shift_;
  //  j["domain"] = da.domain;
}

void from_json(const json &j, DataAxis &da) {
  da.calibration = j["calibration"];
  da.resample_shift_ = j["resample_shift"];
  //  da.domain = j["domain"].get<std::vector<double>>();
}

Dataspace::Dataspace() {}

Dataspace::Dataspace(uint16_t dimensions) : dimensions_(dimensions) {
  axes_.resize(dimensions);
}

Dataspace::Dataspace(const Dataspace &other)
    : axes_(other.axes_), dimensions_(other.dimensions_),
      total_count_(other.total_count_) {}

EntryList Dataspace::all_data() const {
  std::vector<Pair> ranges;
  for (auto a : axes_)
    ranges.push_back(a.bounds());
  return this->range(ranges);
}

DataAxis Dataspace::axis(uint16_t dimension) const {
  if (dimension < axes_.size())
    return axes_[dimension];
  else
    return DataAxis();
}

void Dataspace::set_axis(size_t dim, const DataAxis &ax) {
  if (dim < axes_.size())
    axes_[dim] = ax;
  //  else throw?
}

uint16_t Dataspace::dimensions() const { return dimensions_; }

void Dataspace::load(const hdf5::node::Group &g) {
  using namespace hdf5;

  try {
    auto dgroup = hdf5::node::Group(g).get_group("dataspace");
    axes_.clear();
    if (dgroup.has_group("axes")) {
      for (auto n : dgroup.get_group("axes").nodes) {
        auto ssg = node::Group(n);
        json jj;
        hdf5::to_json(jj, ssg);
        DataAxis ax = jj;
        auto domainds = ssg.get_dataset("domain");
        auto shape =
            dataspace::Simple(domainds.dataspace()).current_dimensions();
        ax.domain.resize(shape[0]);
        domainds.read(ax.domain);
        axes_.push_back(ax);
      }
    }

    this->data_load(node::Group(dgroup["data"]));
  } catch (...) {
    std::throw_with_nested(std::runtime_error("<Dataspace> Could not load"));
  }
}

void Dataspace::save(const hdf5::node::Group &g) const {
  using namespace hdf5;

  try {
    auto dgroup = g.create_group("dataspace");
    if (axes_.size()) {
      auto axes_group = dgroup.create_group("axes");
      for (size_t i = 0; i < axes_.size(); ++i) {
        auto ssg =
            axes_group.create_group(vector_idx_minlen(i, axes_.size() - 1));
        hdf5::from_json(json(axes_[i]), ssg);
        auto &data = axes_[i].domain;
        auto dtype = datatype::create<double>();
        auto dspace = hdf5::dataspace::Simple({data.size()});
        auto domainds = ssg.create_dataset("domain", dtype, dspace);
        domainds.write(data);
      }
    }

    this->data_save(dgroup.create_group("data"));
  } catch (...) {
    std::throw_with_nested(std::runtime_error("<Dataspace> Could not save"));
  }
}

std::string Dataspace::debug(std::string prepend) const {
  std::stringstream ss;
  ss << "DATASPACE\n";

  if (axes_.empty())
    ss << prepend << k_branch_mid_B << "Axes undefined\n";
  else {
    ss << prepend << k_branch_mid_B << "Axes:\n";
    for (size_t i = 0; i < axes_.size(); ++i) {
      ss << prepend << k_branch_pre_B
         << (((i + 1) == axes_.size()) ? k_branch_end_B : k_branch_mid_B) << i
         << "   " << axes_.at(i).debug() << "\n";
    }
  }
  ss << prepend << k_branch_end_B << "Data:\n"
     << this->data_debug(prepend + "  ");
  return ss.str();
}

std::string Dataspace::data_debug(const std::string &) const {
  return std::string();
}

PreciseFloat Dataspace::total_count() const { return total_count_; }

} // namespace DAQuiri
