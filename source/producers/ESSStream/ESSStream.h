#pragma once

#include "producer.h"
#include <atomic>
#include <thread>
#include <librdkafka/rdkafkacpp.h>
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

  StreamManifest stream_manifest() const override;

  bool daq_start(SpillQueue out_queue) override;
  bool daq_stop() override;
  bool daq_running() override;

private:
  //no copying
  void operator=(ESSStream const&);
  ESSStream(const ESSStream&);

  //Acquisition threads, use as static functors
  void worker_run(SpillQueue spill_queue);

private:
  std::atomic<bool> terminate_ {false};
  std::atomic<bool> running_ {false};
  std::thread runner_;

  //Kafka
  std::unique_ptr<RdKafka::KafkaConsumer> stream_;

  // cached params
  std::string kafka_broker_name_;
  std::string kafka_topic_name_;
  uint16_t kafka_timeout_ {1000};
  uint16_t kafka_decomission_wait_ {5000};
  int32_t kafka_max_backlog_ {3};

  std::shared_ptr<fb_parser> parser_;

  uint64_t dropped_buffers_ {0};
  uint64_t clock_ {0};
  double time_spent_ {0};

  uint64_t get_message(SpillQueue);
//  SpillPtr get_message();
  std::string debug(std::shared_ptr<RdKafka::Message> kmessage);
  void select_parser(std::string);

  std::vector<RdKafka::TopicPartition*> get_partitions();
  std::unique_ptr<RdKafka::Metadata> get_kafka_metadata() const;

  void seek(const std::string &topic, uint32_t partition, int64_t offset) const;
};
