#pragma once

#include "plugin.h"
#include "event.h"
#include <random>

using namespace DAQuiri;

class ValueDefinition : public DAQuiri::Plugin
{
  public:
    ValueDefinition();

    std::string plugin_name() const override { return "Value"; }

    Setting settings() const override;
    void settings(const Setting&) override;

    void define(EventModel& def);
    void generate(size_t index, Event& event);

  private:
    std::string name;
    uint32_t max{0};
    double center{0.5};
    double spread{100};
    uint16_t bits_{6};
    std::normal_distribution<double> dist;

    uint32_t trace_size{100};
    uint32_t trace_baseline{0};
    double trace_onset{0.1};
    double trace_risetime{0.2};

    std::default_random_engine gen_;

    uint32_t generate_val();
    void make_trace(size_t index, Event& e, uint32_t val);
};
