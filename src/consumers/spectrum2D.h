#pragma once

#include "spectrum.h"

class Spectrum2D : virtual public Spectrum
{
public:
  Spectrum2D();

protected:
  bool _initialize() override;
  void _init_from_file(std::string name) override;
  void _recalc_axes() override;

  PreciseFloat _data(std::initializer_list<size_t> list) const override;
  std::unique_ptr<EntryList> _data_range(std::initializer_list<Pair> list) override;
  void _append(const Entry&) override;

  void _set_detectors(const std::vector<Detector>& dets) override;

  void _save_data(H5CC::Group&) const override;
  void _load_data(H5CC::Group&) override;
  std::string _data_debug(const std::string& prepend) const override;

  typedef std::map<std::pair<uint16_t,uint16_t>, PreciseFloat> SpectrumMap2D;

  // cached parameters
  uint16_t bits_ {0};

  //the data itself
  SpectrumMap2D spectrum_;
  SpectrumMap2D temp_spectrum_;
  bool buffered_;

  void bin_pair(const uint16_t& x,
                const uint16_t& y,
                const PreciseFloat& count);

  bool is_symmetric();

  static void fill_list(std::unique_ptr<EntryList>& result,
                        const SpectrumMap2D& source,
                        size_t min0, size_t max0,
                        size_t min1, size_t max1);
};
