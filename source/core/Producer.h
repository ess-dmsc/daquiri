/* Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file Producer.h
///
/// \brief Base class for Daquiri producers (which are Kafka consumers)
///
//===----------------------------------------------------------------------===//
#pragma once

#include <core/plugin/Plugin.h>
#include <core/SpillDequeue.h>

namespace DAQuiri {

class Producer;

using ProducerPtr = std::shared_ptr<Producer>;
using SpillQueue = SpillMultiqueue*;
using OscilData = std::map<std::string, Event>;

enum ProducerStatus
{
  dead      = 0,
  loaded    = 1 << 0,
  booted    = 1 << 1,
  running   = 1 << 2,
  can_boot  = 1 << 3,
  can_run   = 1 << 4,
  can_oscil = 1 << 5
};

ProducerStatus operator|(ProducerStatus a, ProducerStatus b);
ProducerStatus operator&(ProducerStatus a, ProducerStatus b);
ProducerStatus operator^(ProducerStatus a, ProducerStatus b);

class Producer : public Plugin
{
  public:
    Producer() = default;

    virtual ~Producer() {}

    ProducerStatus status() const { return status_; }

    virtual void boot() = 0;
    virtual void die() = 0;

    virtual void get_all_settings() {}

    virtual StreamManifest stream_manifest() const { return StreamManifest(); }

    virtual OscilData oscilloscope() { return OscilData(); }

    virtual bool daq_init() { return true; }

    virtual bool daq_start(SpillQueue) { return false; }

    virtual bool daq_stop() { return true; }

    virtual bool daq_running() { return false; }

  protected:
    ProducerStatus status_{ProducerStatus::dead};

    Setting enrich_and_toggle_presets(Setting) const;

};

}
