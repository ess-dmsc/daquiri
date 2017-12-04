#pragma once

#include "producer.h"

using namespace DAQuiri;

class DetectorIndex : public Producer
{
public:
  DetectorIndex();
  ~DetectorIndex();

  std::string plugin_name() const override {return "DetectorIndex";}

  void write_settings_bulk(const Setting&) override;
  void read_settings_bulk(Setting&) const override;
  void boot() override;
  void die() override;

private:
  //no copying
  void operator=(DetectorIndex const&);
  DetectorIndex(const DetectorIndex&);

protected:
  std::vector<Detector> detectors_;

  void add_setting_defs();
};
