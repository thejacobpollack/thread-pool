// @description: See header file.

#include "threadpool.h"

using std::mutex;
using std::size_t;

// See header file.
ThreadPool::ThreadPool(const size_t size) : destroy_(false) {
  // Set the target function for the ready task predicate.
  this->ready_task_ = [this] {
    return this->destroy_ || !this->tasks_.empty();
  };

  // Create the specified number of new worker threads.
  for (size_t k = 0; k < size; ++k) {
    this->workers_.emplace_back([this] {
      // This worker thread's assigned task.
      task_t task;
      // Synchronization state of this worker thread.
      bool synchronize = false;

      // Iterate until this worker thread is to be synchronized.
      do {
        {
          // Block this worker thread until it can accept a task from the 
          // thread pool.
          std::unique_lock<mutex> lock(this->instance_mutex_);

          this->cond_handle_task_.wait(lock, this->ready_task_);

          // Determine if we are destroying this thread pool and the tasks 
          // queue is empty. If so, synchronize this worker thread, otherwise 
          // dequeue a task and assign it to this worker thread.
          if (this->destroy_ && this->tasks_.empty()) {
            synchronize = true;
          } else {
            task = std::move(this->tasks_.front());

            this->tasks_.pop();
          }
        }

        // Determine if we are not synchronizing this worker thread. If so, 
        // invoke this worker thread's assigned task.
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

  // Notify all of the blocked worker threads of this state change.
  this->cond_handle_task_.notify_all();

  // Block the current thread until all of the worker threads from this thread 
  // pool are synchronized.
  for (auto &worker : this->workers_) {
    worker.join();
  }
}