#pragma once

#include <QDateTime>
#include <chrono>

QDateTime fromTimePoint(std::chrono::time_point<std::chrono::system_clock>);

std::chrono::time_point<std::chrono::system_clock> toTimePoint(QDateTime);
