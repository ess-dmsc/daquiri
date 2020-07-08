#pragma once

#include <core/plugin/Plugin.h>
#include <core/Event.h>
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
    std::string name_;
    uint32_t max_ {0};
    double center_ {0.5};
    double spread_ {1000};
    uint16_t bits_ {16};
    uint32_t trace_length_ {100};
    std::normal_distribution<double> dist;

    uint32_t trace_baseline_ {0};
    double trace_onset_ {0.1};
    double trace_risetime_ {0.2};

    std::default_random_engine gen_;

    uint32_t generate_val();
    void make_trace(size_t index, Event& e, uint32_t val);
};
