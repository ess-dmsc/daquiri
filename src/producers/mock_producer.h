#pragma once

#include "producer.h"
#include <random>

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
  bool boot() override;
  bool die() override;

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
  std::string setting_definitions_file_;

  boost::atomic<int> run_status_ {0};
  boost::thread *runner_ {nullptr};

  // cached params
  uint16_t bits_ {6};
  uint32_t spill_interval_ {5};
  double   count_rate_ {10};
  double   lambda_ {0};
  double   dead_ {0};
  double   peak_center_ {0.5};
  double   peak_spread_ {1.0};

  int dummy_selection_{0};

  std::string vname1 {"v1"}, vname2 {"v2"};

  EventModel model_hit;
  uint32_t  resolution_ {0};
  int       event_interval_ {150};

  // runtime
  std::normal_distribution<double> dist_;
  std::default_random_engine gen_;

  uint64_t clock_ {0};

  Spill* get_spill(StatusType t, double seconds);
  Status get_status(int16_t chan, StatusType t);
  void add_hit(Spill&);
  static void make_trace(Event& h, uint16_t baseline);
  uint16_t generate();

  void add_dummy_settings();

};
