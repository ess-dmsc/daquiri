#pragma once

#include "producer.h"
#include <random>

#include <librdkafka/rdkafkacpp.h>

using namespace DAQuiri;

class EventMessage;

class GeometryInterpreter
{
  public:
    GeometryInterpreter() {}
    void add_dimension(std::string name, size_t size);
    EventModel model(const TimeBase& tb);
    void interpret_id(Event& e, size_t val);

  private:
    std::vector<std::string> names_;
    std::list<size_t> bounds_;
};

class KafkaProducer : public Producer
{
public:
  KafkaProducer();
  ~KafkaProducer();

  static std::string plugin_name() {return "KafkaProducer";}
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
  void operator=(KafkaProducer const&);
  KafkaProducer(const KafkaProducer&);

  //Acquisition threads, use as static functors
  static void worker_run(KafkaProducer* callback, SpillQueue spill_queue);

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
