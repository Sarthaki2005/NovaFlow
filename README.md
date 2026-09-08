This is the complete, corrected, and fully formatted README.md file. It includes
the architecture details, benchmarking, and profiling sections that were
truncated previously.

# NovaFlow

**NovaFlow** is a high-performance, low-latency task execution engine for C++17. By implementing a decentralized **Work-Stealing architecture**, NovaFlow eliminates the global lock contention bottlenecks found in traditional thread pools, achieving throughputs exceeding **785,000 tasks per second**.

[![Language](https://img.shields.io/badge/language-C%2B%2B17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Performance](https://img.shields.io/badge/Performance-267x_Faster_than_std::async-green.svg)]()
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

---

## 🚀 Performance Highlights

NovaFlow was benchmarked against `std::async` using the **Google Benchmark** framework. The engine excels in micro-task workloads where scheduling overhead is usually the primary bottleneck.

| Metric | std::async (Baseline) | **NovaFlow** | Speedup |
| :--- | :--- | :--- | :--- |
| **Execution Time (10k tasks)** | 4015 ms | **15.0 ms** | **267x** |
| **Peak Throughput** | 2.5k tasks/s | **785k tasks/s** | **314x** |
| **Imbalanced Workload** | High Latency | **Low Latency** | **Optimized** |

---

## ✨ Core Features

* **Work-Stealing Algorithm:** Decentralized scheduling where idle threads "steal" tasks from the back of other workers' queues to ensure 100% CPU utilization.
* **Contention Minimization:** Per-thread local queues reduce mutex contention, allowing the system to scale linearly with core count.
* **Advanced Type Erasure:** A custom `TaskWrapper` enables the pool to handle move-only types (like `std::packaged_task`), which standard `std::function` cannot support.
* **LIFO/FIFO Hybrid:** Local threads process tasks in LIFO order to improve **cache locality**, while stealing occurs in FIFO order to reduce interference with the owner thread.
* **Future-Based API:** Full support for `std::future`, allowing for seamless result retrieval and exception propagation.

---

## 🛠 Usage

NovaFlow is header-only and easy to integrate into existing C++ projects.

```cpp
#include "WorkStealingPool.hpp"

int main() {
    // Initialize pool with hardware concurrency
    WorkStealingPool pool;

    // 1. Fire-and-forget task
    pool.enqueue([] {
        printf("Task executed by thread %p\n", (void*)std::this_thread::get_id());
    });

    // 2. Task with return value and arguments
    auto future = pool.enqueue([](int a, int b) {
        return a + b;
    }, 10, 20);

    // Retrieve result
    int result = future.get(); // result = 30
    
    return 0;
}
```
## 🏗 Architecture Detail

The Bottleneck Problem

In a standard thread pool, all threads compete for a single global task queue
guarded by a single mutex. As core counts increase, worker threads spend more
CPU cycles waiting to acquire the global lock than executing actual task
payloads.

The NovaFlow Solution

NovaFlow assigns an independent LocalQueue to every worker thread:

1.  Local Push/Pop: When a worker thread enqueues or executes a task, it
    interacts directly with its own local queue without acquiring global locks.
2.  Work-Stealing Mechanism: When a worker thread runs out of tasks in its local
    queue, it transforms into a "thief" thread. It selects a victim thread at
    random and steals tasks from the victim's queue.
3.  Lock Minimization (LIFO/FIFO Hybrid):
      - Local Owner (LIFO): Pushes and pops from the front of its own queue,
        utilizing "hot" CPU cache lines for optimal data locality.
      - Stealer Threads (FIFO): Steal tasks from the back of the victim's queue.
        This dual-ended approach minimizes synchronization lock contention
        between local execution and external stealing.

## 📈 Benchmarking & Systems Profiling

Google Benchmark Suite

Micro-benchmarks conducted across uniform execution loads and imbalanced core
distribution tests:

Benchmark                           Time         CPU   Throughput
--------------------------------------------------------------------
BM_WorkStealing_MicroTasks/10000    15.0 ms     14.1 ms   710k/s
BM_WorkStealing_Imbalanced/50000    63.7 ms     63.7 ms   785k/s
BM_StdAsync_Baseline/10000          4015 ms     3948 ms   2.5k/s

Linux perf Profiling

Low-level hardware performance counters were monitored using perf stat to
evaluate cache locality and CPU efficiency:

perf stat -e task-clock,context-switches,cpu-migrations,cycles,instructions ./bench_pool

## ⚙️ Building & Running

Requirements

  - Compiler: C++17 compatible (GCC 7+, Clang 5+, MSVC 2017+)
  - Build System: CMake 3.14+
  - Environment: Linux / WSL2 / macOS

Build Steps

# Clone the repository
git clone https://github.com/yourusername/NovaFlow.git
cd NovaFlow

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make

# Run the benchmark
./bench_pool

Created by [Sarthaki Bhoir]

