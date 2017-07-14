#include "spectrum1D.h"

Spectrum1D::Spectrum1D()
{
  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata("1D", "1D Spectrum", 1);

  SettingMeta app("appearance", SettingType::color);
  app.set_val("description", "Plot appearance");
  base_options.branches.add(Setting(app));

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

  metadata_.overwrite_all_attributes(base_options);
}

bool Spectrum1D::_initialize()
{
  Spectrum::_initialize();
  bits_ = metadata_.get_attribute("resolution").selection();

  size_t size = pow(2, bits_);
  if (spectrum_.size() < size)
    spectrum_.resize(size, PreciseFloat(0));

  return false; //still too abstract
}

void Spectrum1D::_init_from_file(std::string name)
{
  metadata_.set_attribute(Setting::integer("resolution", bits_), false);
  Spectrum::_init_from_file(name);
}

void Spectrum1D::_set_detectors(const std::vector<Detector>& dets)
{
  metadata_.detectors.resize(metadata_.dimensions(), Detector());

  if (dets.size() == metadata_.dimensions())
    metadata_.detectors = dets;

  if (dets.size() >= metadata_.dimensions())
  {
    for (size_t i=0; i < dets.size(); ++i)
    {
      if (metadata_.chan_relevant(i))
      {
        metadata_.detectors[0] = dets[i];
        break;
      }
    }
  }

  this->_recalc_axes();
}

void Spectrum1D::_recalc_axes()
{
  Spectrum::_recalc_axes();

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

PreciseFloat Spectrum1D::_data(std::initializer_list<size_t> list) const
{
  if (list.size() != 1)
    return 0;
  
  uint32_t chan = *list.begin();
  if (chan >= spectrum_.size())
    return 0;
  else
    return spectrum_[chan];
}

std::unique_ptr<EntryList> Spectrum1D::_data_range(std::initializer_list<Pair> list)
{
  size_t min {0};
  size_t max {spectrum_.size() - 1};
  if (list.size() == 1)
  {
    min = list.begin()->first;
    max = list.begin()->second;
  }

  if (max >= spectrum_.size())
    max = spectrum_.size() - 1;

  if (min >= spectrum_.size())
    min = 0;

  std::unique_ptr<EntryList> result(new EntryList);
  if (spectrum_.empty())
    return result;

  for (size_t i=min; i <= max; i++)
  {
    Entry newentry;
    newentry.first.resize(1, 0);
    newentry.first[0] = i;
    newentry.second = spectrum_[i];
    result->push_back(newentry);
  }

  return result;
}

void Spectrum1D::_append(const Entry& e)
{
  if (e.first.size() && (e.first.at(0) < spectrum_.size()))
  {
    spectrum_[e.first.at(0)] += e.second;
    total_count_ += e.second;
//    total_coincidences_ += e.second;
  }
}

void Spectrum1D::_save_data(H5CC::Group& g) const
{
  std::vector<long double> d(maxchan_);
  for (uint32_t i = 0; i < maxchan_; i++)
    d[i] = static_cast<long double>(spectrum_[i]);
  auto dset = g.require_dataset<long double>("data", {maxchan_});
  dset.write(d);
}

void Spectrum1D::_load_data(H5CC::Group& g)
{
  if (!g.has_dataset("data"))
    return;
  H5CC::DataSet dset = g.open_dataset("data");
  H5CC::Shape shape = dset.shape();
  if (shape.rank() != 1)
    return;

  std::vector<long double> rdata(shape.dim(0));
  dset.read(rdata, {rdata.size()}, {0});

  spectrum_.clear();
  spectrum_.resize(rdata.size(), PreciseFloat(0));
  for (size_t i = 0; i < rdata.size(); i++)
    spectrum_[i] = PreciseFloat(rdata[i]);

  maxchan_ = rdata.size();
}

std::string Spectrum1D::_data_debug(const std::string &prepend) const
{
  std::stringstream ss;

  uint64_t total  = static_cast<uint64_t>(total_count_);
  uint64_t nstars = static_cast<uint64_t>(maxchan_*2);
  if (!nstars)
    nstars = 100;

  for (uint32_t i = 0; i < maxchan_; i++)
  {
    long double val = static_cast<long double>(spectrum_[i]);
    if (val)
      ss << prepend << i << ": " <<
            std::string(val*nstars/total,'*') << "\n";
  }

  return ss.str();
}
