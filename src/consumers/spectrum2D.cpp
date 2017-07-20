#include "spectrum2D.h"

#include "custom_logger.h"
#include "ascii_tree.h"

Spectrum2D::Spectrum2D()
{
  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata("2D", "2D Spectrum", 2);

  SettingMeta resm("resolution", SettingType::menu);
  resm.set_flag("preset");
  resm.set_enum(4, "4 bit (16)");
  resm.set_enum(5, "5 bit (32)");
  resm.set_enum(6, "6 bit (64)");
  resm.set_enum(7, "7 bit (128)");
  resm.set_enum(8, "8 bit (256)");
  resm.set_enum(9, "9 bit (512)");
  resm.set_enum(10, "10 bit (1024)");
  resm.set_enum(11, "11 bit (2048)");
  resm.set_enum(12, "12 bit (4096)");
  resm.set_enum(13, "13 bit (8192)");
  resm.set_enum(14, "14 bit (16384)");
  resm.set_enum(15, "15 bit (32768)");
  resm.set_enum(16, "16 bit (65536)");
  Setting res(resm);
  res.set_number(14);
  base_options.branches.add(res);

  SettingMeta buf("buffered", SettingType::boolean);
  buf.set_val("description", "Buffered output for efficient plotting (more memory)");
  buf.set_flag("preset");
  base_options.branches.add(buf);

  SettingMeta sym("symmetric", SettingType::boolean);
  sym.set_val("description", "Matrix is symmetric");
  sym.set_flag("readonly");
  base_options.branches.add(sym);

  metadata_.overwrite_all_attributes(base_options);
}

bool Spectrum2D::_initialize()
{
  Spectrum::_initialize();
  bits_ = metadata_.get_attribute("resolution").selection();
  buffered_ = metadata_.get_attribute("buffered").triggered();

  return false; //still too abstract
}

void Spectrum2D::_init_from_file(std::string name)
{
  metadata_.set_attribute(Setting::integer("resolution", bits_));
  metadata_.set_attribute(Setting::boolean("buffered", false));
  metadata_.set_attribute(Setting::boolean("symmetric", is_symmetric()));
  Spectrum::_init_from_file(name);
}

void Spectrum2D::_set_detectors(const std::vector<Detector>& dets)
{
  metadata_.detectors.resize(metadata_.dimensions(), Detector());

  if (dets.size() == metadata_.dimensions())
    metadata_.detectors = dets;
  else if (dets.size() > metadata_.dimensions())
  {
    int j=0;
    for (size_t i=0; i < dets.size(); ++i)
    {
      if (metadata_.chan_relevant(i))
      {
        metadata_.detectors[j] = dets[i];
        j++;
        if (j >= metadata_.dimensions())
          break;
      }
    }
  }

  this->_recalc_axes();
}

void Spectrum2D::_recalc_axes()
{
  Spectrum::_recalc_axes();

  size_t res = pow(2,bits_);
  axes_[0].resize(res);
  axes_[1].resize(res);
  for (size_t i=0; i < res; ++i)
    axes_[1][i] = axes_[0][i] = i;

  if (axes_.size() != metadata_.detectors.size())
    return;

  for (size_t i=0; i < metadata_.detectors.size(); ++i)
  {
//    Calibration this_calib = metadata_.detectors[i].best_calib(bits_);
//    uint32_t res = pow(2,bits_);
//    axes_[i].resize(res, 0.0);
//    for (uint32_t j=0; j<res; j++)
//      axes_[i][j] = this_calib.transform(j, bits_);
  }
}

PreciseFloat Spectrum2D::_data(std::initializer_list<size_t> list) const
{
  if (list.size() != 2)
    return 0;

  std::vector<uint16_t> coords(list.begin(), list.end());

  std::pair<uint16_t,uint16_t> point(coords[0], coords[1]);

  if (spectrum_.count(point))
    return spectrum_.at(point);
  else
    return 0;
}

std::unique_ptr<EntryList> Spectrum2D::_data_range(std::initializer_list<Pair> list)
{
  size_t min0, min1, max0, max1;
  if (list.size() != 2)
  {
    min0 = min1 = 0;
    max0 = max1 = pow(2, bits_);
  }
  else
  {
    Pair range0 = *list.begin();
    Pair range1 = *(list.begin()+1);
    min0 = std::min(range0.first, range0.second);
    max0 = std::max(range0.first, range0.second);
    min1 = std::min(range1.first, range1.second);
    max1 = std::max(range1.first, range1.second);
  }

  std::unique_ptr<EntryList> result(new EntryList);
//  CustomTimer makelist(true);

  if (buffered_ && !temp_spectrum_.empty())
    fill_list(result, temp_spectrum_, min0, max0, min1, max1);
  else
    fill_list(result, spectrum_, min0, max0, min1, max1);

  if (!temp_spectrum_.empty())
  {
    boost::unique_lock<boost::mutex> uniqueLock(unique_mutex_, boost::defer_lock);
    while (!uniqueLock.try_lock())
      boost::this_thread::sleep_for(boost::chrono::seconds{1});
    temp_spectrum_.clear(); //assumption about client
  }
//  DBG << "<Spectrum2D> Making list for " << metadata_.name << " took " << makelist.ms() << "ms filled with "
//         << result->size() << " elements";
  return result;
}

void Spectrum2D::fill_list(std::unique_ptr<EntryList>& result,
                           const SpectrumMap2D& source,
                           size_t min0, size_t max0,
                           size_t min1, size_t max1)
{
  for (const auto& it : source)
  {
    size_t co0 = it.first.first;
    size_t co1 = it.first.second;
    if ((min0 <= co0) &&
        (co0 <= max0) &&
        (min1 <= co1) &&
        (co1 <= max1))
    {
      Entry newentry;
      newentry.first.resize(2, 0);
      newentry.first[0] = co0;
      newentry.first[1] = co1;
      newentry.second = it.second;
      result->push_back(newentry);
    }
  }
}

void Spectrum2D::bin_pair(const uint16_t& x, const uint16_t& y,
                          const PreciseFloat& count)
{
  spectrum_[std::pair<uint16_t, uint16_t>(x,y)] += count;
  if (buffered_)
    temp_spectrum_[std::pair<uint16_t, uint16_t>(x, y)] =
        spectrum_[std::pair<uint16_t, uint16_t>(x, y)];
  total_count_ += count;
}

void Spectrum2D::_append(const Entry& e)
{
  if (e.first.size() == 2)
    bin_pair(e.first[0], e.first[1], e.second);
}

void Spectrum2D::_save_data(H5CC::Group& g) const
{
  auto dgroup = g.require_group("data");
  auto didx = dgroup.require_dataset<uint16_t>("indices", {spectrum_.size(), 2}, {128,2});
  auto dcts = dgroup.require_dataset<long double>("counts", {spectrum_.size()}, {128});
  std::vector<uint16_t> dx(spectrum_.size());
  std::vector<uint16_t> dy(spectrum_.size());
  std::vector<long double> dc(spectrum_.size());
  size_t i = 0;
  for (auto it = spectrum_.begin(); it != spectrum_.end(); ++it)
  {
    dx[i] = it->first.first;
    dy[i] = it->first.second;
    dc[i] = static_cast<long double>(it->second);
    i++;
  }
  didx.write(dx, {spectrum_.size(), 1}, {0,0});
  didx.write(dy, {spectrum_.size(), 1}, {0,1});
  dcts.write(dc);
}

void Spectrum2D::_load_data(H5CC::Group& g)
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
  std::vector<long double> dc(didx.shape().dim(0));

  didx.read(dx, {dx.size(), 1}, {0,0});
  didx.read(dy, {dy.size(), 1}, {0,1});
  dcts.read(dc, {dx.size()}, {0});

  for (size_t i=0; i < dx.size(); ++i)
    spectrum_[std::pair<uint16_t, uint16_t>(dx[i],dy[i])] = dc[i];
}

std::string Spectrum2D::_data_debug(const std::string &prepend) const
{
  double maximum {0};
  for (auto &b : spectrum_)
    maximum = std::max(maximum, to_double(b.second));

  std::string representation(ASCII_grayscale94);
  std::stringstream ss;

  uint16_t res = pow(2, bits_);

  ss << "Maximum=" << maximum << "\n";
  for (uint16_t i = 0; i < res; i++)
  {
    for (uint16_t j = 0; j < res; j++)
    {
      uint16_t v = 0;
      if (spectrum_.count(std::pair<uint16_t, uint16_t>(i,j)))
        v = spectrum_.at(std::pair<uint16_t, uint16_t>(i,j));
      ss << representation[v / maximum * 93];
    }
    ss << "\n";
  }

  return ss.str();
}

bool Spectrum2D::is_symmetric()
{
  bool symmetric = true;
  for (auto &q : spectrum_)
  {
    std::pair<uint16_t,uint16_t> point(q.first.second, q.first.first);
    if ((!spectrum_.count(point)) || (spectrum_.at(point) != q.second))
    {
      symmetric = false;
      break;
    }
  }
  return symmetric;
}
