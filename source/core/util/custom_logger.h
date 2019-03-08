#pragma once

#include <iostream>
#include <string>

#include <fmt/format.h>
#include <cstdint>
#include <libgen.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <graylog_logger/Log.hpp>
#pragma GCC diagnostic pop

#pragma GCC system_header

namespace CustomLogger {
  void initLogger(Log::Severity severity, std::ostream *gui_stream, std::string log_file_N);
  void closeLogger();
};

#define LOG(Severity, Format, ...) Log::Msg(Severity, fmt::format(Format, ##__VA_ARGS__))

#define CRIT(Format, ...) LOG(Log::Severity::Critical, Format, ##__VA_ARGS__)
#define ERR(Format, ...) LOG(Log::Severity::Error, Format, ##__VA_ARGS__)
#define WARN(Format, ...) LOG(Log::Severity::Warning, Format, ##__VA_ARGS__)
#define INFO(Format, ...) LOG(Log::Severity::Informational, Format, ##__VA_ARGS__)
#define DBG(Format, ...) LOG(Log::Severity::Debug, Format, ##__VA_ARGS__)

#define LOGL(Severity, Format, ...) Log::Msg(Severity, fmt::format(Format, ##__VA_ARGS__), {{"file", std::string(__FILE__)}, {"line", std::int64_t(__LINE__)}})

#define CRITL(Format, ...) LOGL(Log::Severity::Critical, Format, ##__VA_ARGS__)
#define ERRL(Format, ...) LOGL(Log::Severity::Error, Format, ##__VA_ARGS__)
#define WARNL(Format, ...) LOGL(Log::Severity::Warning, Format, ##__VA_ARGS__)
#define INFOL(Format, ...) LOGL(Log::Severity::Info, Format, ##__VA_ARGS__)
#define DBGL(Format, ...) LOGL(Log::Severity::Debug, Format, ##__VA_ARGS__)
