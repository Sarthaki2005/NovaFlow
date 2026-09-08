This is a professional README.md file designed to showcase your engineering
skills to recruiters and senior developers. It uses industry-standard formatting
and highlights the high-performance nature of your work.

# NovaFlow

**NovaFlow** is a high-performance, low-latency task execution engine for C++17. By implementing a decentralized **Work-Stealing architecture**, NovaFlow eliminates the global lock contention bottlenecks found in traditional thread pools, achieving throughputs exceeding **785,000 tasks per second**.

[![Language](https://img.shields.io/badge/language-C%2B%2B17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Performance](https://img.shields.io/badge/Performance-267x_Faster_than_std::async-green.svg)]()

---

## 🚀 Performance Highlights

NovaFlow was benchmarked against `std::async` using the **Google Benchmark** framework. The engine excels in micro-task workloads where scheduling overhead is usually the primary bottleneck.

| Metric | std::async (Baseline) | **NovaFlow** | Speedup |
| :--- | :--- | :--- | :--- |
| **Execution Time (10k tasks)** | 4015 ms | **15.0 ms** | **267x** |
| **Peak Throughput** | 2.5k tasks/s | **785k tasks/s** | **314x** |
| **Imbalanced Workload** | High Latency | **Low Latency** | **Optimized** |

## ✨ Core Features

*   **Work-Stealing Algorithm:** Decentralized scheduling where idle threads "steal" tasks from the back of other workers' queues to ensure 100% CPU utilization.
*   **Contention Minimization:** Per-thread local queues reduce mutex contention, allowing the system to scale linearly with core count.
*   **Advanced Type Erasure:** A custom `TaskWrapper` enables the pool to handle move-only types (like `std::packaged_task`), which standard `std::function` cannot support.
*   **LIFO/FIFO Hybrid:** Local threads process tasks in LIFO order to improve **cache locality**, while stealing occurs in FIFO order to reduce interference with the owner thread.
*   **Future-Based API:** Full support for `std::future`, allowing for seamless result retrieval and exception propagation.

## 🛠 Usage

NovaFlow is header-only and easy to integrate into existing C++ projects.

```cpp
#include "WorkStealingPool.hpp"

int main() {
    // Initialize pool with hardware concurrency (e.g., 4 or 8 threads)
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

🏗 Architecture Detail

The Bottleneck Problem

In a standard thread pool, all threads compete for a single global task queue.
As core counts increase, the time spent waiting for the global lock often
exceeds the time spent executing the actual task.

The NovaFlow Solution

NovaFlow assigns a LocalQueue to every worker thread:

1.  Local Push/Pop: When a thread enqueues a task, it pushes it to its own
    queue. It also pops from its own queue first.
2.  The Steal: If a thread's queue is empty, it becomes a "thief" and looks at
    other threads' queues. It steals from the bottom of the victim's queue.
3.  Synchronization: This "Top-Bottom" approach minimizes the chance of a worker
    and a thief trying to access the same memory simultaneously.

📈 Benchmarking Results

The following results were captured on a 4-core machine:

Benchmark                          Time             CPU   Throughput
--------------------------------------------------------------------
BM_WorkStealing_MicroTasks/10000   15.0 ms         14.1 ms   710k/s
BM_WorkStealing_Imbalanced/50000   63.7 ms         63.7 ms   785k/s
BM_StdAsync_Baseline/10000         4015 ms         3948 ms   2.5k/s

⚙️ Build Requirements

  - Compiler: C++17 compatible (GCC 7+, Clang 5+, MSVC 2017+)
  - Build System: CMake 3.10+
  - Dependencies: Header-only (No external dependencies)

Created by [Your Name]

