#pragma once

#include "producer.h"
#include "ess_geometry.h"
#include <librdkafka/rdkafkacpp.h>

using namespace DAQuiri;

class EventMessage;

class ESSStream : public Producer
{
public:
  ESSStream();
  ~ESSStream();

  static std::string plugin_name() {return "ESSStream";}
  std::string device_name() const override {return plugin_name();}

  void write_settings_bulk(const Setting&) override;
  void read_settings_bulk(Setting&) const override;
  void boot() override;
  void die() override;

  bool daq_start(SpillQueue out_queue) override;
  bool daq_stop() override;
  bool daq_running() override;

private:
  //no copying
  void operator=(ESSStream const&);
  ESSStream(const ESSStream&);

  //Acquisition threads, use as static functors
  static void worker_run(ESSStream* callback, SpillQueue spill_queue);

protected:
  boost::atomic<int> run_status_ {0};
  boost::thread *runner_ {nullptr};

  //Kafka
  std::unique_ptr<RdKafka::KafkaConsumer> consumer_;

  // cached params
  std::string broker_;
  std::string topic_;
  int poll_interval_ {1};

  uint32_t spill_interval_ {5};

  std::string detector_type_;
  EventModel model_hit_;

  uint64_t clock_ {0};
  uint64_t buf_id_ {0};

  Status get_status(int16_t chan, StatusType t);
  static void make_trace(Event& h, uint16_t baseline);

  Spill* get_message();
  Spill* process_message(std::shared_ptr<RdKafka::Message> msg);
  Spill* create_spill(StatusType t);

  std::string debug(const EventMessage&);
  void interpret_id(Event& e, size_t val);
};
