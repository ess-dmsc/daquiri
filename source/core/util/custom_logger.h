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
  void initLogger(Severity severity, std::ostream *gui_stream, std::string log_file_N);
  void closeLogger();
};

#define LOG(Severity, Format, ...) Log::Msg(Severity, fmt::format(Format, ##__VA_ARGS__))

#define CRIT(Format, ...) LOG(Severity::Critical, Format, ##__VA_ARGS__)
#define ERR(Format, ...) LOG(Severity::Error, Format, ##__VA_ARGS__)
#define WARN(Format, ...) LOG(Severity::Warning, Format, ##__VA_ARGS__)
#define INFO(Format, ...) LOG(Severity::Informational, Format, ##__VA_ARGS__)
#define DBG(Format, ...) LOG(Severity::Debug, Format, ##__VA_ARGS__)

#define LOGL(Severity, Format, ...) Log::Msg(Severity, fmt::format(Format, ##__VA_ARGS__), {{"file", std::string(__FILE__)}, {"line", std::int64_t(__LINE__)}})

#define CRITL(Format, ...) LOGL(Severity::Critical, Format, ##__VA_ARGS__)
#define ERRL(Format, ...) LOGL(Severity::Error, Format, ##__VA_ARGS__)
#define WARNL(Format, ...) LOGL(Severity::Warning, Format, ##__VA_ARGS__)
#define INFOL(Format, ...) LOGL(Severity::Informational, Format, ##__VA_ARGS__)
#define DBGL(Format, ...) LOGL(Severity::Debug, Format, ##__VA_ARGS__)
