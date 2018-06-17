#pragma once

#include <core/producer.h>

namespace DAQuiri {

class DummyDevice : public Producer
{
  public:
    DummyDevice();
    ~DummyDevice();

    std::string plugin_name() const override { return "DummyDevice"; }

    void settings(const Setting&) override;
    Setting settings() const override;

    void boot() override;
    void die() override;

    StreamManifest stream_manifest() const override;

  protected:
    StreamManifest manifest_;

    int dummy_selection_{0};

    bool read_only_{false};

    void add_dummy_settings();
};

}