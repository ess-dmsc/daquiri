/* Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file logger.h
///
/// \brief logging macros - uses spdlog
/// despite
//===----------------------------------------------------------------------===//
#pragma once

#include <iostream>
#include <string>

#include <fmt/format.h>
#include <cstdint>
#include <libgen.h>

#include <cassert> /// Somehow spdlog fails to bring it along on its own
#include <spdlog/spdlog.h>

#pragma GCC system_header

namespace CustomLogger
{

void initLogger(const spdlog::level::level_enum& LoggingLevel,
                const std::string& log_file_name,
                std::ostream* gui_stream = nullptr);
void closeLogger();

}

// Do not use directly use the defines below instead
// global loglevel is set in main.cpp, daquiri.cpp, ...
#define LOG(Severity, Format, ...) spdlog::log(Severity, Format, ##__VA_ARGS__)

/// \brief macros to be used in your code
/// \todo align with efu nomenclature INFO -> INF, WARN->WAR, etc.
#define CRIT(Format, ...) LOG(spdlog::level::critical, Format, ##__VA_ARGS__)
#define ERR(Format, ...) LOG(spdlog::level::err, Format, ##__VA_ARGS__)
#define WARN(Format, ...) LOG(spdlog::level::warn, Format, ##__VA_ARGS__)
#define INFO(Format, ...) LOG(spdlog::level::info, Format, ##__VA_ARGS__)
#define DBG(Format, ...) LOG(spdlog::level::debug, Format, ##__VA_ARGS__)
#define TRC(Format, ...) LOG(spdlog::level::trace, Format, ##__VA_ARGS__)
