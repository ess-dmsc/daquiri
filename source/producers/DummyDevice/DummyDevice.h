#pragma once

#include <core/producer.h>

namespace DAQuiri
{

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

  integer_t int_unbounded_{0};
  integer_t int_lower_bounded_{0};
  integer_t int_upper_bounded_{0};
  integer_t int_bounded_{0};

  floating_t float_unbounded_{0};
  floating_t float_lower_bounded_{0};
  floating_t float_upper_bounded_{0};
  floating_t float_bounded_{0};

  precise_t precise_unbounded_{0};
  precise_t precise_lower_bounded_{0};
  precise_t precise_upper_bounded_{0};
  precise_t precise_bounded_{0};

  hr_time_t time_{};
  hr_duration_t duration_{};

  Pattern pattern_;
  bool bool_ {false};
  integer_t binary_ {7};

  std::string text_;
  std::string color_;
  std::string file_;
  std::string directory_;
  std::string detector_;
  std::string gradient_;
};

}