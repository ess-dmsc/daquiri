/* Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file      */
//===----------------------------------------------------------------------===//
///
/// \file Configuration.cpp
///
//===----------------------------------------------------------------------===//

#include <Configuration.h>
#include <fmt/format.h>

void Configuration::print() {
  fmt::print("[Kafka]\n");
  fmt::print("  Broker {}\n", Kafka.Broker);
  fmt::print("  Topic {}\n", Kafka.Topic);
  fmt::print("[Geometry]\n");
  fmt::print("  Dimensions ({}, {}, {})\n", Geometry.XDim, Geometry.YDim, Geometry.ZDim);
  fmt::print("[Plot]\n");
  fmt::print("  Clear periodically {}\n", Plot.ClearPeriodic);
  fmt::print("  Clear interval (s) {}\n", Plot.ClearEverySeconds);
  fmt::print("  Interpolate image {}\n", Plot.Interpolate);
  fmt::print("  Color gradient {}\n", Plot.ColorGradient);
  fmt::print("  Invert gradient {}\n", Plot.InvertGradient);
  fmt::print("  Log Scale {}\n", Plot.LogScale);
  fmt::print("  Title {}\n", Plot.Title);
}

void Configuration::fromJsonFile(std::string fname)
{
  std::ifstream ifs(fname, std::ofstream::in);
  nlohmann::json j;
  if (!ifs.good()) {
    throw("Invalid json configuration file, exiting ...");
  }
  ifs >> j;

  /// 'geometry' field is mandatory
  try {
    Geometry.XDim = j["geometry"]["xdim"];
    Geometry.YDim = j["geometry"]["ydim"];
    Geometry.ZDim = j["geometry"]["zdim"];
  } catch (...) {
    throw std::runtime_error("Config error: invalid 'geometry' field");
  }

  /// 'kafka' field is mandatory. 'broker' and 'topic' must be specified
  try {
    Kafka.Broker = j["kafka"]["broker"];
    Kafka.Topic = j["kafka"]["topic"];
  } catch (...) {
    throw std::runtime_error("Config error in 'kafka' field: missing/bad values of broker or topic");
  }

  /// The rest are optional, so we just use default values if there is an
  /// invalid/missing configuration
  try {
    Kafka.MessageMaxBytes = j["kafka"]["message.max.bytes"];
    Kafka.FetchMessagMaxBytes = j["kafka"]["fetch.message.max.bytes"];
    Kafka.ReplicaFetchMaxBytes  = j["kafka"]["replica.fetch.max.bytes"];
    Kafka.EnableAutoCommit= j["kafka"]["enable.auto.commit"];
    Kafka.EnableAutoOffsetStore= j["kafka"]["enable.auto.offset.store"];

    Plot.ClearPeriodic = j["plot"]["clear_periodic"];
    Plot.ClearEverySeconds = j["plot"]["clear_interval_seconds"];
    Plot.Interpolate = j["plot"]["interpolate_pixels"];
    Plot.ColorGradient = j["plot"]["color_gradient"];
    Plot.InvertGradient = j["plot"]["invert_gradient"];
    Plot.LogScale = j["plot"]["log_scale"];
    Plot.Title = j["plot"]["title"];
  } catch (...) {
    fmt::print("Noncritical error in configuration - using default values\n");
  }
  print();
}
