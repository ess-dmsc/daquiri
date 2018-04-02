#pragma once

#include "producer.h"
#include <atomic>
#include <thread>
#include "fb_parser.h"
#include "KafkaPlugin.h"

using namespace DAQuiri;

class ESSStream : public Producer
{
  public:
    ESSStream();
    ~ESSStream();

    std::string plugin_name() const override { return "ESSStream"; }

    void settings(const Setting&) override;
    Setting settings() const override;

    void boot() override;
    void die() override;

    StreamManifest stream_manifest() const override;

    bool daq_start(SpillQueue out_queue) override;
    bool daq_stop() override;
    bool daq_running() override;

  private:
    //no copying
    void operator=(ESSStream const &);
    ESSStream(const ESSStream &);

    //Acquisition threads, use as static functors
    void worker_run(SpillQueue spill_queue);

  private:
    std::map<std::string, int32_t> parser_names_;

    std::atomic<bool> terminate_{false};
    std::atomic<bool> running_{false};
    std::thread runner_;

    // cached params
    // topic-level
    KafkaConfigPlugin kafka_config_;
    std::string kafka_topic_name_;
    bool kafka_ff_{false};
    int32_t kafka_max_backlog_{3};

    // Kafka
    std::shared_ptr<RdKafka::KafkaConsumer> stream_;

    // Flatbuffer parser
    std::shared_ptr<fb_parser> parser_;

    uint64_t dropped_buffers_{0};
    uint64_t clock_{0};
    double time_spent_{0};

    uint64_t get_message(SpillQueue);
    void select_parser(std::string);

    void ff_stream(std::shared_ptr<RdKafka::Message> message);
};
