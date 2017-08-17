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
  std::unique_ptr<RdKafka::KafkaConsumer> stream_;

  // cached params
  std::string kafka_broker_name_;
  std::string kafka_topic_name_;
  int kafka_timeout_ {1};

  std::string detector_type_;
  size_t dim_count_ {1};
  GeometryInterpreter geometry_;
  EventModel model_hit_;

  uint64_t clock_ {0};
  uint64_t buf_id_ {0};

  Status get_status(int16_t chan, StatusType t);
  static void make_trace(Event& h, uint16_t baseline);

  Spill* get_message();
  Spill* process_message(std::shared_ptr<RdKafka::Message> msg);
  Spill* create_spill(StatusType t);

  std::string debug(const EventMessage&);
};
