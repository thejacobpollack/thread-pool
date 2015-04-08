// @description: See header file.

#include "threadpool.h"

using std::mutex;
using std::size_t;

// See header file.
ThreadPool::ThreadPool(const size_t size) : destroy_(false) {
  // Creates the specified number of new worker threads.
  for (size_t k = 0; k < size; ++k) {
    this->workers_.emplace_back([this] {
      // Stores this worker thread's assigned task.
      task_t task;
      // Determines if we need to synchronize this worker thread.
      bool synchronize = false;

      // Iterate until this worker thread is signalled to be synchronized.
      do {
        {
          // Block the current worker thread until this thread pool is being 
          // destroyed or there is a task to be invoked.
          std::unique_lock<mutex> lock(this->instance_mutex_);

          this->cond_handle_task_.wait(lock, [this] {
            return (this->destroy_ || !this->tasks_.empty());
          });

          // Determines if we are destroying this thread pool and the tasks 
          // queue is empty. If so, synchronize this worker thread, otherwise 
          // dequeue the next task and assign it to this worker thread.
          if (this->destroy_ && this->tasks_.empty()) {
            synchronize = true;
          } else {
            task = std::move(this->tasks_.front());

            this->tasks_.pop();
          }
        }

        // Determines if we are not synchronizing this worker. If so, invoke 
        // this worker thread's assigned task.
        if (!synchronize) {
          try {
            task();
          } catch (std::exception &) {
            // Log the exception caught by the worker thread's assigned task.
          } catch (...) { }
        }
      } while (!synchronize);
    });
  }
}

// See header file.
ThreadPool::~ThreadPool() {
  // Change the state of the thread pool to "being destroyed".
  {
    std::lock_guard<mutex> lock(this->instance_mutex_);

    this->destroy_ = true;
  }

  // Notify all of the queued worker threads of this state change.
  this->cond_handle_task_.notify_all();

  // Block the current thread until all of the worker threads from this thread 
  // pool have been synchronized with it.
  for (auto &worker : this->workers_) {
    worker.join();
  }
}