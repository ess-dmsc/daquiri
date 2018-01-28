#pragma once

#include "producer.h"

using namespace DAQuiri;

class DummyDevice : public Producer
{
public:
  DummyDevice();
  ~DummyDevice();

  std::string plugin_name() const override {return "DummyDevice";}

  void write_settings_bulk(const Setting&) override;
  void read_settings_bulk(Setting&) const override;
  void boot() override;
  void die() override;

  StreamManifest stream_manifest() const override;

private:
  //no copying
  void operator=(DummyDevice const&);
  DummyDevice(const DummyDevice&);

protected:
  StreamManifest manifest_;

  int dummy_selection_{0};

  bool read_only_ {false};

  void add_dummy_settings();
};
