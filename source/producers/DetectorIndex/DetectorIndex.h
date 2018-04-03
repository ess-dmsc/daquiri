#pragma once

#include "producer.h"

using namespace DAQuiri;

class DetectorIndex : public Producer
{
  public:
    DetectorIndex();
    ~DetectorIndex();

    std::string plugin_name() const override { return "DetectorIndex"; }

    void settings(const Setting&) override;
    Setting settings() const override;

    void boot() override;
    void die() override;

  protected:
    std::vector<Detector> detectors_;

    void define_settings();
};
