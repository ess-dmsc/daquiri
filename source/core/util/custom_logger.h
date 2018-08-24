#pragma once

#include <iostream>
#include <string>

#include <fmt/format.h>
#include <cstdint>
#include <libgen.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <graylog_logger/GraylogInterface.hpp>
#include <graylog_logger/FileInterface.hpp>
#include <graylog_logger/ConsoleInterface.hpp>
#include <graylog_logger/Log.hpp>
#pragma GCC diagnostic pop

#pragma GCC system_header

enum class Sev : int {
  Emergency = 0,
  Alert = 1,
  Critical = 2,
  Error = 3,
  Warning = 4,
  Notice = 5,
  Info = 6,
  Debug = 7,
};

inline int SevToInt(Sev Level) { // Force the use of the correct type
  return static_cast<int>(Level);
}

namespace CustomLogger {
  void initLogger(Sev severity, std::ostream *gui_stream, std::string log_file_N);
  void closeLogger();
};

#define LOG(Severity, Format, ...) Log::Msg(SevToInt(Severity), fmt::format(Format, ##__VA_ARGS__), {{"file", std::string(__FILE__)}, {"line", std::int64_t(__LINE__)}})

#define CRIT(Format, ...) LOG(Sev::Critical, Format, ##__VA_ARGS__)
#define ERR(Format, ...) LOG(Sev::Error, Format, ##__VA_ARGS__)
#define WARN(Format, ...) LOG(Sev::Warning, Format, ##__VA_ARGS__)
#define INFO(Format, ...) LOG(Sev::Info, Format, ##__VA_ARGS__)
#define DBG(Format, ...) LOG(Sev::Debug, Format, ##__VA_ARGS__)
