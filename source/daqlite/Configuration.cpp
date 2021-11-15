/* Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file      */
//===----------------------------------------------------------------------===//
///
/// \file Configuration.cpp
///
//===----------------------------------------------------------------------===//

#include <Configuration.h>
#include <fmt/format.h>


void Configuration::fromJsonFile(std::string fname) {
  std::ifstream ifs(fname, std::ofstream::in);
  if (!ifs.good()) {
    throw("Invalid json configuration file, exiting ...");
  }
  ifs >> j;

  /// 'geometry' field is mandatory
  Geometry.XDim = getInt("geometry", "xdim", Geometry.XDim, true);
  Geometry.YDim = getInt("geometry", "ydim", Geometry.YDim, true);
  Geometry.ZDim = getInt("geometry", "zdim", Geometry.ZDim, true);
  Geometry.Offset = getInt("geometry", "offset", Geometry.Offset);

  /// 'kafka' field is mandatory. 'broker' and 'topic' must be specified
  Kafka.Broker = getString("kafka", "broker", "n/a", true);
  Kafka.Topic = getString("kafka", "topic", "n/a", true);
  /// The rest are optional, so we just use default values if there is an
  /// invalid/missing configuration
  Kafka.MessageMaxBytes = getString("kafka", "message.max.bytes", Kafka.MessageMaxBytes);
  Kafka.FetchMessagMaxBytes = getString("kafka", "fetch.message.max.bytes", Kafka.FetchMessagMaxBytes);
  Kafka.ReplicaFetchMaxBytes = getString("kafka", "replica.fetch.max.bytes", Kafka.ReplicaFetchMaxBytes);
  Kafka.EnableAutoCommit = getString("kafka", "enable.auto.commit", Kafka.EnableAutoCommit);
  Kafka.EnableAutoOffsetStore = getString("kafka", "enable.auto.offset.store", Kafka.EnableAutoOffsetStore);

  // Plot options - all are optional
  Plot.PlotType = getString("plot", "plot_type", Plot.PlotType);
  Plot.ClearPeriodic = getBool("plot", "clear_periodic", Plot.ClearPeriodic);
  Plot.ClearEverySeconds = getInt("plot", "clear_interval_seconds", Plot.ClearEverySeconds);
  Plot.Interpolate = getBool("plot", "interpolate_pixels", Plot.Interpolate);
  Plot.ColorGradient = getString("plot", "color_gradient", Plot.ColorGradient);
  Plot.InvertGradient = getBool("plot", "invert_gradient", Plot.InvertGradient);
  Plot.LogScale = getBool("plot", "log_scale", Plot.LogScale);

  // Window options - all are optional
  Plot.WindowTitle = getString("plot", "window_title", Plot.WindowTitle);
  Plot.PlotTitle = getString("plot", "plot_title", Plot.PlotTitle);
  Plot.XAxis = getString("plot", "xaxis", Plot.XAxis);
  Plot.Width = getInt("plot", "window_width", Plot.Width);
  Plot.Height = getInt("plot", "window_height", Plot.Height);

  TOF.Scale = getInt("tof", "scale", TOF.Scale);
  TOF.MaxValue = getInt("tof", "max_value", TOF.MaxValue);
  TOF.BinSize = getInt("tof", "bin_size", TOF.BinSize);
  TOF.AutoScale = getBool("tof", "auto_scale", TOF.AutoScale);

  print();
}

void Configuration::print() {
  fmt::print("[Kafka]\n");
  fmt::print("  Broker {}\n", Kafka.Broker);
  fmt::print("  Topic {}\n", Kafka.Topic);
  fmt::print("[Geometry]\n");
  fmt::print("  Dimensions ({}, {}, {})\n", Geometry.XDim, Geometry.YDim, Geometry.ZDim);
  fmt::print("  Pixel Offset {}\n", Geometry.Offset);
  fmt::print("[Plot]\n");
  fmt::print("  WindowTitle {}\n", Plot.WindowTitle);
  fmt::print("  Plot type {}\n", Plot.PlotType);
  fmt::print("  Clear periodically {}\n", Plot.ClearPeriodic);
  fmt::print("  Clear interval (s) {}\n", Plot.ClearEverySeconds);
  fmt::print("  Interpolate image {}\n", Plot.Interpolate);
  fmt::print("  Color gradient {}\n", Plot.ColorGradient);
  fmt::print("  Invert gradient {}\n", Plot.InvertGradient);
  fmt::print("  Log Scale {}\n", Plot.LogScale);
  fmt::print("  PlotTitle {}\n", Plot.PlotTitle);
  fmt::print("  X Axis {}\n", Plot.XAxis);
  fmt::print("[TOF]\n");
  fmt::print("  Scale {}\n", TOF.Scale);
  fmt::print("  Max value {}\n", TOF.MaxValue);
  fmt::print("  Bin size {}\n", TOF.BinSize);
  fmt::print("  Auto scale {}\n", TOF.AutoScale);
}


int Configuration::getInt(std::string Group, std::string Option, int Default,
    bool Throw) {
  int ConfigVal;
  try {
    ConfigVal = j[Group][Option];
  } catch (nlohmann::json::exception& e) {
    fmt::print("Missing [{}][{}] configuration\n", Group, Option);
    if (Throw) {
      throw std::runtime_error("Daqlite config error");
    } else {
      fmt::print("Using default: {}\n", Default);
      return Default;
    }
  }
  return ConfigVal;
}

bool Configuration::getBool(std::string Group, std::string Option, bool Default,
    bool Throw) {
      bool ConfigVal;
      try {
        ConfigVal = j[Group][Option];
      } catch (nlohmann::json::exception& e) {
        fmt::print("Missing [{}][{}] configuration\n", Group, Option);
        if (Throw) {
          throw std::runtime_error("Daqlite config error");
        } else {
          fmt::print("Using default: {}\n", Default);
          return Default;
        }
      }
      return ConfigVal;
}

std::string Configuration:: getString(std::string Group, std::string Option,
    std::string Default, bool Throw) {
  std::string ConfigVal;
  try {
    ConfigVal = j[Group][Option];
  } catch (nlohmann::json::exception& e) {
    fmt::print("Missing [{}][{}] configuration\n", Group, Option);
    if (Throw) {
      throw std::runtime_error("Daqlite config error");
    } else {
      fmt::print("Using default: {}\n", Default);
      return Default;
    }
  }
  return ConfigVal;
}
