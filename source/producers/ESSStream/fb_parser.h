#pragma once

#include "producer.h"
#include "custom_timer.h"

using namespace DAQuiri;

class KafkaStreamConfig : public Plugin
{
  public:
    KafkaStreamConfig();

    std::string plugin_name() const override { return "KafkaTopicConfig"; }
    Setting settings() const override;
    void settings(const Setting&) override;

    // topic-level params
    std::string kafka_topic_name_;
    bool kafka_ff_{false};
    int64_t kafka_max_backlog_{3};
};

class EventMessage;

class fb_parser : public Producer
{
  public:
    struct PayloadStats
    {
        uint64_t time_start {0};
        uint64_t time_end {0};
        double time_spent {0};
        uint64_t dropped_buffers {0};
    };

    PayloadStats stats;
    KafkaStreamConfig kafka_config;

    fb_parser();
    virtual ~fb_parser() {}

    void settings(const Setting&) override;
    Setting settings() const override;

    void boot() override;
    void die() override;

    virtual uint64_t process_payload(SpillQueue spill_queue, void* msg) = 0;
    virtual uint64_t stop(SpillQueue spill_queue) = 0;
};
