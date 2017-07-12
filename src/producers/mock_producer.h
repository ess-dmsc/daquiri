#pragma once

#include "producer.h"
#include <boost/random/discrete_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>

using namespace DAQuiri;

class MockProducer : public Producer
{
public:
  MockProducer();
  ~MockProducer();

  static std::string plugin_name() {return "MockProducer";}
  std::string device_name() const override {return plugin_name();}

  void write_settings_bulk(Setting &set) override;
  void read_settings_bulk(Setting &set) const override;
  void get_all_settings() override;
  bool boot() override;
  bool die() override;

  bool daq_start(SynchronizedQueue<Spill*>* out_queue) override;
  bool daq_stop() override;
  bool daq_running() override;

private:
  //no copying
  void operator=(MockProducer const&);
  MockProducer(const MockProducer&);

  //Acquisition threads, use as static functors
  static void worker_run(MockProducer* callback, SynchronizedQueue<Spill*>* spill_queue);

protected:
  std::string setting_definitions_file_;

  boost::atomic<int> run_status_ {0};
  boost::thread *runner_ {nullptr};

  // cached params
  int      bits_ {6};
  uint32_t spill_interval_ {5};
  double   count_rate_ {5000};
  double   lambda_ {0};
  int      event_interval_ {10};
  uint64_t resolution_;
  EventModel model_hit;

  // runtime
  boost::random::discrete_distribution<> dist_;
  boost::random::mt19937 gen;
  double lab_time {0};
  double live_time {0};
  uint64_t clock_ {0};

  Spill get_spill();
  Status getBlock(double duration);
  void push_hit(Spill&, uint16_t);
  static void make_trace(Event& h, uint16_t baseline);
};
