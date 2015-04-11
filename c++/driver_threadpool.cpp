// @description: Test driver for the thread pool module.

#include <iostream>
#include <functional>
#include <future>

#include "threadpool.h"

using std::cout;
using std::endl;

int main() {
  // Create a thread pool with 5 worker threads.
  ThreadPool pool(5);
  // Store the result of 10 distinct tasks.
  std::future<int> results[10];
  
  // Enqueue 10 tasks without arguments onto the thread pool.
  for (int i = 0; i < 10; ++i) {
    results[i] = pool.enqueue([i] {
      return i;
    });
  }
  
  // Iterate over each task in the same order they were enqueued as and output 
  // the result.
  for (int i = 0; i < 10; ++i) {
    cout << "Task[" << i << "] returned: " << results[i].get() << endl;
  }

  // ===========================================================================
  cout << "---" << endl;
  // ===========================================================================

  // Enqueue 10 tasks with arguments onto the thread pool.
  for (int i = 0; i < 10; ++i) {
    results[i] = pool.enqueue([] (const int i) {
      return i;
    }, i);
  }

  // Iterate over each task in the same order they were enqueued as and output 
  // the result.
  for (int i = 0; i < 10; ++i) {
    cout << "Task[" << i << "] returned: " << results[i].get() << endl;
  }

  // ===========================================================================
  cout << "---" << endl;
  // ===========================================================================

  // Passing lvalue arguments and using references.
  int i = 4;

  cout << "BEFORE: i = " << i << endl;

  results[0] = pool.enqueue([] (int &k) {
    k += 1;

    return k;
  }, std::ref(i));

  cout << "Task[0] returned: " << results[0].get() << endl;
  cout << "AFTER: i = " << i << endl;
}