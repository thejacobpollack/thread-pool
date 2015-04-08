// @description: Stores an arbitrary number of threads and facilitates the 
//               queueing and distribution of arbitrary tasks to various 
//               threads pooled in the thread pool. In the event that a task 
//               throws an exception, if the exception type is derived from 
//               std::exception then it will be logged, otherwise ignored, and 
//               the worker thread will return to the thread pool. Undefined 
//               behaviour occurs whenever an operation on the mutex object 
//               throws an exception.

#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

class ThreadPool final {
public:
  // Creates a thread pool with the specified size and initializes each worker 
  // thread. Provides basic exception safety.
  explicit ThreadPool(const std::size_t size);
  // Destroys a thread pool. Must be called from a thread not managed by this 
  // thread pool.
  ~ThreadPool();
  // Enqueues a task onto the tasks queue. Provides basic exception safety.
  template<class Fn, class ...ArgTypes>
  auto enqueue(Fn &&fn, ArgTypes && ...args) -> std::future<decltype(fn(args...))>;
private:
  // The type of a task.
  using task_t = std::function<void()>;
  // Copy constructor disabled.
  ThreadPool(const ThreadPool &) = delete;
  // Move constructor disabled.
  ThreadPool(ThreadPool &&) = delete;
  // Copy assignment operator disabled.
  ThreadPool &operator=(const ThreadPool &) = delete;
  // Move assignment operator disabled.
  ThreadPool &operator=(ThreadPool &&) = delete;
  // Stores a list of worker threads.
  std::vector<std::thread> workers_;
  // Stores a queue of tasks to be distributed and invoked.
  std::queue<task_t> tasks_;
  // Stores a mutex for atomically getting and setting this thread pool's 
  // fields.
  std::mutex instance_mutex_;
  // Stores the condition used to lock each worker thread until notified to 
  // invoke its assigned task.
  std::condition_variable cond_handle_task_;
  // State of the thread pool.
  bool destroy_;
};

// See declaration above.
template<class Fn, class ...ArgTypes> 
inline auto ThreadPool::enqueue(Fn &&fn, ArgTypes && ...args) -> std::future<decltype(fn(args...))> {
  // Package the specified task in a new copyable managed object and preserve 
  // each argument's value category.
  auto task = std::make_shared<std::packaged_task<decltype(fn(args...))()>>(
    std::bind(std::forward<Fn>(fn), std::forward<ArgTypes>(args)...)
  );
  // Get the future return value of the specified task.
  auto retval = task->get_future();

  // Enqueue the specified task.
  {
    std::lock_guard<std::mutex> lock(this->instance_mutex_);

    // Determine if we are preparing to destroy this thread pool. If so, throw 
    // a runtime error exception.
    if (this->destroy_) {
      throw std::runtime_error("Cannot enqueue a task while this thread pool "
                               "is preparing to be destroyed.");
    }
    
    this->tasks_.emplace([task = std::move(task)] { (*task)(); });
  }

  // Notify a worker thread that there is a new task to be invoked.
  this->cond_handle_task_.notify_one();

  return retval;
}

#endif