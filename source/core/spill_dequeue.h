#pragma once

#include "spill.h"
#include <deque>
#include <condition_variable>
#include <map>
#include <mutex>

#include "custom_logger.h"
#include <boost/range/adaptor/map.hpp>

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
    bool push(const SpillPtr& s, bool drop, size_t max_buffers)
    {
      if (drop &&
          (s->type == StatusType::running) &&
          (recent_running_spills >= max_buffers))
        return true;

      if (s->type == StatusType::running)
        recent_running_spills++;
      if (earliest.is_not_a_date_time())
        earliest = s->time;
      queue.push_back(s);
      return false;
    }

    SpillPtr pop()
    {
      SpillPtr f = queue.front();
      if (f->type == StatusType::running)
        recent_running_spills--;
      queue.pop_front();
      if (queue.size())
        earliest = queue.front()->time;
      else
        earliest = boost::posix_time::not_a_date_time;
      return f;
    }

    size_t size() const
    {
      return queue.size();
    }

    boost::posix_time::ptime earliest;
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

  inline void enqueue(const SpillPtr& data)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    SmartSpillDeque& stream = streams_[data->stream_id];

    if (stream.push(data, drop_, max_buffers_))
    {
      DBG << "<SpillMultiqueue> Dropped " << data->to_string();
      dropped_++;
    }
    size_++;

    cond_.notify_one();
  }

  inline SpillPtr dequeue()
  {
    std::unique_lock<std::mutex> lock(mutex_);

    while (!size_ && !stop_)
      cond_.wait(lock);

    if (stop_ || !size_)
      return nullptr;

    SmartSpillDeque* earliest = &streams_[""];
    for (auto& s : streams_)
    {
      if (s.second.size() &&
          (earliest->earliest.is_not_a_date_time() ||
           (earliest->earliest > s.second.earliest)))
        earliest = &s.second;
    }

    SpillPtr result = earliest->pop();
    size_--;

    return result;
  }

  inline void stop()
  {
    stop_ = true;
    cond_.notify_all();
  }

  inline size_t size()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    return size_;
  }

  inline size_t dropped()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    return dropped_;
  }

private:
  std::mutex mutex_;
  std::condition_variable cond_;
  bool stop_ {false};

  std::map<std::string, SmartSpillDeque> streams_;
//  SmartSpillDeque& earliest_ {streams_[""]};

  size_t size_ {0};
  size_t dropped_ {0};

  bool drop_ {false};
  size_t max_buffers_ {10};
};


}
