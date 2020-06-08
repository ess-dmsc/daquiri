/* Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file fb_parser.h
///
/// \brief Abstract class for parsing flatbuffer messages
///
//===----------------------------------------------------------------------===//
#pragma once

#include <core/Producer.h>
#include <core/util/Timer.h>

using namespace DAQuiri;

class EventMessage;

class fb_parser : public Producer
{
 public:
  struct PayloadStats
  {
    uint64_t time_start{0};
    uint64_t time_end{0};
    double time_spent{0};
    uint64_t dropped_buffers{0};
  };

  PayloadStats stats;

  fb_parser();

  virtual ~fb_parser() = default;

  virtual std::string schema_id() const = 0;
  virtual std::string get_source_name(void* msg) const = 0;

  // \todo function to validate buffer

  void boot() override;
  void die() override;

  virtual uint64_t process_payload(SpillMultiqueue * spill_queue, void* msg) = 0;
  virtual uint64_t stop(SpillMultiqueue * spill_queue) = 0;
};

using FBParserPtr = std::shared_ptr<fb_parser>;
