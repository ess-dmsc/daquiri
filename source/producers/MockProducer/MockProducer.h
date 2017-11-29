#pragma once

#include "producer.h"
#include <random>

#include <atomic>
#include <thread>

using namespace DAQuiri;

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
  static void worker_run(MockProducer* callback, SpillQueue spill_queue);

protected:
  std::atomic<int> run_status_ {0};
  std::thread *runner_ {nullptr};

  // cached params
  uint16_t bits_ {6};
  uint32_t spill_interval_ {5};
  double   count_rate_ {10};
  double   lambda_ {0};
  double   spill_lambda_ {100};
  double   dead_ {0};
  std::string stream_id;

  size_t val_count_ {1};
  std::vector<std::string> vnames_;
  std::vector<double> centers_;
  std::vector<double> spreads_;
  std::vector<std::normal_distribution<double>> dists_;

  EventModel model_hit;

  uint32_t  resolution_ {0};

  // runtime
//  std::normal_distribution<double> dist_;
  std::default_random_engine gen_;

  uint64_t clock_ {0};
  uint64_t recent_pulse_time_ {0};

  SpillPtr get_spill(StatusType t, double seconds);
  void fill_stats(Spill& spill) const;
  void add_hit(Spill&);
  static void make_trace(Event& h, uint16_t baseline);
  uint16_t generate(size_t i);
};
