#pragma once

#include <shared_mutex>
#include <thread>
#include "custom_timer.h"

#define SLEEP_TIME std::chrono::microseconds(100)

using mutex = std::mutex;
using mutex_st = std::shared_timed_mutex;
using shared_lock_st = std::shared_lock<mutex_st>;
using unique_lock_st = std::unique_lock<mutex_st>;
using unique_lock = std::unique_lock<mutex>;

#define UNIQUE_LOCK unique_lock ulock(mutex_, std::defer_lock);
#define UNIQUE_LOCK_ST unique_lock_st ulock(mutex_, std::defer_lock);
#define SHARED_LOCK_ST shared_lock_st lock(mutex_);
#define EVENTUALLY_LOCK while (!ulock.try_lock()) std::this_thread::sleep_for(SLEEP_TIME);
#define UNIQUE_LOCK_EVENTUALLY UNIQUE_LOCK EVENTUALLY_LOCK
#define UNIQUE_LOCK_EVENTUALLY_ST UNIQUE_LOCK_ST EVENTUALLY_LOCK
