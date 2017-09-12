#pragma once

#include "producer.h"
#include "ess_geometry.h"
#include <librdkafka/rdkafkacpp.h>

#include <atomic>
#include <thread>
#include "fb_parser.h"


using namespace DAQuiri;

class EventMessage;
class GEMHist;
class GEMTrack;

class ESSStream : public Producer
{
public:
  ESSStream();
  ~ESSStream();

  std::string plugin_name() const override {return "ESSStream";}

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

private:
  std::atomic<int> run_status_ {0};
  std::thread *runner_ {nullptr};

  //Kafka
  std::unique_ptr<RdKafka::KafkaConsumer> stream_;

  // cached params
  std::string kafka_broker_name_;
  std::string kafka_topic_name_;
  int kafka_timeout_ {1000};
  int spoof_clock_ {0};

  TimeBase time_base_;
  std::shared_ptr<fb_parser> parser_;

  uint64_t clock_ {0};
  double time_spent_ {0};

  SpillPtr get_message();
  std::string debug(std::shared_ptr<RdKafka::Message> kmessage);
  void select_parser(std::string);
};
