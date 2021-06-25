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
  fmt::print("  Plot type {}\n", Plot.PlotType);
  fmt::print("  Clear periodically {}\n", Plot.ClearPeriodic);
  fmt::print("  Clear interval (s) {}\n", Plot.ClearEverySeconds);
  fmt::print("  Interpolate image {}\n", Plot.Interpolate);
  fmt::print("  Color gradient {}\n", Plot.ColorGradient);
  fmt::print("  Invert gradient {}\n", Plot.InvertGradient);
  fmt::print("  Log Scale {}\n", Plot.LogScale);
  fmt::print("  Title {}\n", Plot.Title);
  fmt::print("  X Axis {}\n", Plot.XAxis);
  fmt::print("[TOF]\n");
  fmt::print("  Scale {}\n", TOF.Scale);
  fmt::print("  Max value {}\n", TOF.MaxValue);
  fmt::print("  Bin size {}\n", TOF.BinSize);
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
  } catch (nlohmann::json::exception& e) {
    fmt::print("{}\n", e.what());
    throw std::runtime_error("Config error: invalid 'geometry' field");
  }

  /// 'kafka' field is mandatory. 'broker' and 'topic' must be specified
  try {
    Kafka.Broker = j["kafka"]["broker"];
    Kafka.Topic = j["kafka"]["topic"];
  } catch (nlohmann::json::exception& e) {
    fmt::print("{}\n", e.what());
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
  } catch (nlohmann::json::exception& e) {
    fmt::print("Noncritical error in Kafka configuration - using default values\n");
    fmt::print("{}\n", e.what());
  }

try {
    Plot.PlotType = j["plot"]["plot_type"];
    Plot.ClearPeriodic = j["plot"]["clear_periodic"];
    Plot.ClearEverySeconds = j["plot"]["clear_interval_seconds"];
    Plot.Interpolate = j["plot"]["interpolate_pixels"];
    Plot.ColorGradient = j["plot"]["color_gradient"];
    Plot.InvertGradient = j["plot"]["invert_gradient"];
    Plot.LogScale = j["plot"]["log_scale"];
    Plot.Title = j["plot"]["title"];
    Plot.XAxis = j["plot"]["xaxis"];
    Plot.Width = j["plot"]["window_width"];
    Plot.Height = j["plot"]["window_height"];
  } catch (nlohmann::json::exception& e) {
    fmt::print("Noncritical error in Plot configuration - using default values\n");
    fmt::print("{}\n", e.what());
  }

try {
    TOF.Scale = j["tof"]["scale"];
    TOF.MaxValue = j["tof"]["max_value"];
    TOF.BinSize = j["tof"]["bin_size"];
  } catch (nlohmann::json::exception& e) {
    fmt::print("Noncritical error in TOF configuration - using default values\n");
    fmt::print("{}\n", e.what());
  }
  print();
}
