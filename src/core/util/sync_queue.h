#pragma once

#include <queue>	
//#include <boost/thread.hpp>
#include <condition_variable>
#include <mutex>

template <typename T>
class SynchronizedQueue
{
public:
  inline SynchronizedQueue() : end_queue_(false) {}
  
  inline void enqueue(const T& data)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    
    queue_.push(data);
    
    cond_.notify_one();
  }
  
  inline T dequeue()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    
    while (queue_.empty() && !end_queue_)
      cond_.wait(lock);
    
    if (end_queue_)
      return NULL;
    
    T result = queue_.front();
    queue_.pop();
    
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
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cond_;
};
