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

    KafkaConfigPlugin kafka_config_;

    Kafka::ConsumerPtr stream_;
    std::shared_ptr<fb_parser> parser_;

    uint64_t get_message(SpillQueue);
    void select_parser(std::string);

    static bool good(Kafka::MessagePtr message);
    static uint64_t ff_stream(Kafka::ConsumerPtr consumer,
                              Kafka::MessagePtr message,
                              int64_t kafka_max_backlog);
};
