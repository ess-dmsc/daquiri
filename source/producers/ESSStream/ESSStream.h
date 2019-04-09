#pragma once

#include <core/producer.h>
#include <atomic>
#include <thread>
#include <producers/ESSStream/fb_parser.h>
#include <producers/ESSStream/KafkaPlugin.h>

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

  private:
    std::map<std::string, int32_t> parser_names_;

    std::atomic<bool> running_{false};
    std::atomic<bool> terminate_{false};

    KafkaConfigPlugin kafka_config_;

    struct Stream
    {
      KafkaStreamConfig config;
      Kafka::ConsumerPtr consumer;
      FBParserPtr parser;
      std::thread runner;

      void worker_run(SpillQueue spill_queue, uint16_t consume_timeout,
                      std::atomic<bool>* terminate);

      uint64_t ff_stream(Kafka::MessagePtr message, int64_t kafka_max_backlog);
    };

    std::vector<Stream> streams_;

    void select_parser(size_t, std::string);

    static bool good(Kafka::MessagePtr message);

    static std::string get_fb_id(Kafka::MessagePtr message);
};
