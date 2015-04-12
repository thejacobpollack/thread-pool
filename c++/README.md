# Thread Pool in C++14

The following package contains an implementation of a simple thread pool module 
in C++14. It does not capitalize on thread affinity, data locality, and other 
optimizations.

Used GNU's Compiler Collection version 4.9.2 with the C++14 extension enabled.

## Usage

Basic usage is as follows:

```c++
// Create a thread pool with 10 worker threads.
ThreadPool pool(10);

// Enqueue a closure -- referred to as a task.
auto result = pool.enqueue([] { return 42; });

// Get the promised result from the future.
int fourty_two = result.get();
```
