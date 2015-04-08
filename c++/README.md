# Thread Pool in C++14

The following package contains an implementation of a simple thread pool in 
C++14. It does not capitalize on thread affinity, data locality, and other 
optimizations.

Used GNU's Compiler Collection version 4.9.2 with the C++14 extension enabled.

## Usage

Basic usage is as follows:

```c++
ThreadPool pool(10);

auto result = pool.enqueue([] { return 42; });

cout << result.get() << endl;
```