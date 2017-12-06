#pragma once

#include "producer.h"
#include <random>

#include <atomic>
#include <thread>

using namespace DAQuiri;

struct ValueDefinition
{
    std::string name;
    uint32_t max {0};
    double center {0.5};
    double spread {100};
    std::normal_distribution<double> dist;

    uint32_t trace_size {500};
    uint32_t trace_baseline {0};
    double trace_onset {0.1};
    double trace_risetime {0.2};

    void define(EventModel& def);
    void generate(size_t index, Event& event, std::default_random_engine& gen);

    uint32_t generate(std::default_random_engine& gen);
    void make_trace(size_t index, Event& e, uint32_t val);
};


class MockProducer : public Producer
{
public:
  MockProducer();
  ~MockProducer();

  std::string plugin_name() const override {return "MockProducer";}

  void write_settings_bulk(const Setting&) override;
  void read_settings_bulk(Setting&) const override;
  void boot() override;
  void die() override;

  bool daq_start(SpillQueue out_queue) override;
  bool daq_stop() override;
  bool daq_running() override;

private:
  //no copying
  void operator=(MockProducer const&);
  MockProducer(const MockProducer&);

  //Acquisition threads, use as static functors
  void worker_run(SpillQueue spill_queue);

protected:
  std::atomic<bool> terminate_ {false};
  std::atomic<bool> running_ {false};
  std::thread runner_;

  // cached params
  uint16_t bits_ {6};
  double spill_interval_ {1};
  double count_rate_ {10};
  double lambda_ {0};
  double spill_lambda_ {100};
  double dead_ {0};
  std::string stream_id;

  std::vector<ValueDefinition> val_defs_ {1, ValueDefinition()};

  EventModel event_definition_;

  // runtime
  std::default_random_engine gen_;

  uint64_t clock_ {0};
  uint64_t recent_pulse_time_ {0};

  SpillPtr get_spill(StatusType t, double seconds);
  void fill_stats(Spill& spill) const;
  void add_hit(Spill&);
};
