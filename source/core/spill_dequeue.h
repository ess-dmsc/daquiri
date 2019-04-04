#pragma once

#include <core/spill.h>
#include <deque>
#include <condition_variable>
#include <map>
#include <mutex>
#include <atomic>

#include <core/util/logger.h>

namespace DAQuiri {

class SpillDeque
{
public:
  inline SpillDeque() : end_queue_(false) {}
  
  inline void enqueue(const SpillPtr& data)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push_back(data);
    cond_.notify_one();
  }
  
  inline SpillPtr dequeue()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    
    while (queue_.empty() && !end_queue_)
      cond_.wait(lock);
    
    if (end_queue_)
      return nullptr;
    
    SpillPtr result = queue_.front();
    queue_.pop_front();
    
    return result;
  }
  
  inline void stop()
  {
    end_queue_ = true;
    cond_.notify_all();        
  }

  inline uint32_t size()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.size();
  }
  
private:
  bool end_queue_;
  std::deque<SpillPtr> queue_;
  std::mutex mutex_;
  std::condition_variable cond_;
};

struct SmartSpillDeque
{
    // returns true if spill is dropped
    bool push(const SpillPtr& s, bool drop, size_t max_buffers)
    {
      if (drop &&
          (s->type == Spill::Type::running) &&
          (recent_running_spills >= max_buffers))
        return true;

      if (s->type == Spill::Type::running)
        recent_running_spills++;

      if (earliest == hr_time_t())
        earliest = s->time;

      queue.push_back(s);

      return false;
    }

    // Must be non-empty
    SpillPtr pop()
    {
      SpillPtr f = queue.front();
      queue.pop_front();

      if (f->type == Spill::Type::running)
        recent_running_spills--;

      if (queue.size())
        earliest = queue.front()->time;
      else
        earliest = hr_time_t();

      return f;
    }

    size_t size() const
    {
      return queue.size();
    }

    bool empty() const
    {
      return queue.empty();
    }

    hr_time_t earliest;
    std::deque<SpillPtr> queue;
    size_t recent_running_spills {0};
};


class SpillMultiqueue
{
public:
  inline SpillMultiqueue(bool drop, size_t max_buffers)
    : drop_(drop)
    , max_buffers_(max_buffers)
  {}

  // will enqueue to appropriate stream
  inline void enqueue(const SpillPtr& data)
  {
    std::unique_lock<std::mutex> lock(mutex_);

    if (streams_[data->stream_id].push(data, drop_, max_buffers_))
    {
      dropped_spills_ ++;
      dropped_events_ += data->events.size();
      return;
    }

    // only do this if enqeued properly
    size_++;
    cond_.notify_one();
  }

  // return nullptr if terminating
  inline SpillPtr dequeue()
  {
    std::unique_lock<std::mutex> lock(mutex_);

    // will not release if empty
    while (!size_ && !stop_)
      cond_.wait(lock);

    // this is the end...
    if (stop_)
      return nullptr;

    // selecting earliest ensures chronological queue
    SmartSpillDeque* earliest = &streams_[""];
    for (auto& s : streams_)
    {
      if (!s.second.empty() &&
          ((earliest->earliest == hr_time_t())
          || (earliest->earliest > s.second.earliest)))
        earliest = &s.second;
    }

    size_--;
    return earliest->pop();
  }

  inline void stop()
  {
    stop_ = true;
    cond_.notify_all();
  }

  inline size_t size()
  {
    return size_.load();
  }

  inline size_t dropped_spills()
  {
    return dropped_spills_.load();
  }

  inline size_t dropped_events()
  {
    return dropped_events_.load();
  }

private:
  std::mutex mutex_;
  std::condition_variable cond_;
  bool stop_ {false};

  std::map<std::string, SmartSpillDeque> streams_;

  std::atomic<size_t> size_ {0};
  std::atomic<size_t> dropped_spills_ {0};
  std::atomic<size_t> dropped_events_ {0};

  bool drop_ {false};
  size_t max_buffers_ {10};
};


}
