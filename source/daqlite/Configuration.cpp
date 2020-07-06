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
  fmt::print("  Clear inerval (s) {}\n", Plot.ClearEverySeconds);
  fmt::print("  Interpolate image {}\n", Plot.Interpolate);
  fmt::print("  Invert gradient {}\n", Plot.InvertGradient);
  fmt::print("  Title {}\n", Plot.Title);
}

void Configuration::fromJsonFile(std::string fname)
{
  std::ifstream ifs(fname, std::ofstream::in);
  nlohmann::json j;
  if (!ifs.good()) {
    throw("Invalid configuration file, exiting ...");
  }
  ifs >> j;

  Geometry.XDim = j["geometry"]["xdim"];
  Geometry.YDim = j["geometry"]["ydim"];
  Geometry.ZDim = j["geometry"]["zdim"];

  Kafka.Broker = j["kafka"]["broker"];
  Kafka.Topic = j["kafka"]["topic"];

  Plot.ClearPeriodic = j["plot"]["clear_periodic"];
  Plot.ClearEverySeconds = j["plot"]["clear_interval_seconds"];
  Plot.Interpolate = j["plot"]["interpolate_pixels"];
  Plot.InvertGradient = j["plot"]["invert_gradient"];
  Plot.Title = j["plot"]["title"];
}
