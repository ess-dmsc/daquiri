#pragma once

#include <QDateTime>
#include <chrono>

QDateTime fromTimePoint(std::chrono::time_point<std::chrono::high_resolution_clock>);

std::chrono::time_point<std::chrono::high_resolution_clock> toTimePoint(QDateTime);
