#pragma once

#include <queue>	
#include <boost/thread.hpp>

template <typename T>
class SynchronizedQueue
{
public:
  inline SynchronizedQueue() : end_queue_(false) {}
  
  inline void enqueue(const T& data)
  {
    boost::unique_lock<boost::mutex> lock(mutex_);
    
    queue_.push(data);
    
    cond_.notify_one();
  }
  
  inline T dequeue()
  {
    boost::unique_lock<boost::mutex> lock(mutex_);
    
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
    boost::unique_lock<boost::mutex> lock(mutex_);
    return queue_.size();
  }
  
private:
  bool end_queue_;
  std::queue<T> queue_;
  boost::mutex mutex_;
  boost::condition_variable cond_;
};
