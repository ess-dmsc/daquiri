#pragma once

#include <core/producer.h>
#include "ValueDefinition.h"

#include <atomic>
#include <thread>

using namespace DAQuiri;

class MockProducer : public Producer
{
  public:
    MockProducer();
    ~MockProducer();

    std::string plugin_name() const override { return "MockProducer"; }

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
    void operator=(MockProducer const&);
    MockProducer(const MockProducer&);

    //Acquisition threads, use as static functors
    void worker_run(SpillQueue spill_queue);

  protected:
    std::atomic<bool> terminate_{false};
    std::atomic<bool> running_{false};
    std::thread runner_;

    // cached params
    std::string stream_id_;
    double spill_interval_{1};  // seconds between spills
    double count_rate_{10};     // events per second
    double lambda_{0};          // total decay constant
    double spill_lambda_{100};  // decay constant per spill
    double dead_{0};            // percent dead time

    std::vector<ValueDefinition> val_defs_{1, ValueDefinition()};

    EventModel event_definition_;

    // runtime
    std::default_random_engine gen_;
    std::uniform_real_distribution<> event_chance_{0, 1};

    uint64_t clock_{0};
    uint64_t recent_pulse_time_{0};

    SpillPtr get_spill(StatusType t, double seconds);
    void fill_events(SpillPtr& spill, double seconds);
    void fill_stats(Spill& spill) const;
    bool eval_spill_lambda(uint32_t i, uint32_t total);
    void add_hit(Spill&, uint64_t time);
};
