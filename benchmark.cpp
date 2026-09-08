#include <benchmark/benchmark.h>
#include <future>
#include <vector>
#include <cmath>
#include "ThreadPool.cpp" // Ensure your pool class is in a header

// -------------------------------------------------------------
// Benchmark 1: High-Contention Micro-Tasks (100k fast tasks)
// -------------------------------------------------------------
static void BM_WorkStealing_MicroTasks(benchmark::State& state) {
    const size_t num_tasks = state.range(0);

    for (auto _ : state) {
        WorkStealingPool pool(std::thread::hardware_concurrency());
        std::vector<std::future<int>> futures;
        futures.reserve(num_tasks);

        for (size_t i = 0; i < num_tasks; ++i) {
            futures.push_back(pool.enqueue([](int x) { return x * x; }, static_cast<int>(i)));
        }

        for (auto& f : futures) {
            benchmark::DoNotOptimize(f.get()); // Prevent compiler dead-code elimination
        }
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_WorkStealing_MicroTasks)->Arg(10000)->Arg(100000)->Unit(benchmark::kMillisecond);

// -------------------------------------------------------------
// Benchmark 2: Non-Uniform / Imbalanced Workload
// -------------------------------------------------------------
static void BM_WorkStealing_Imbalanced(benchmark::State& state) {
    const size_t num_tasks = state.range(0);

    for (auto _ : state) {
        WorkStealingPool pool(std::thread::hardware_concurrency());
        std::vector<std::future<double>> futures;
        futures.reserve(num_tasks);

        for (size_t i = 0; i < num_tasks; ++i) {
            futures.push_back(pool.enqueue([i]() {
                // Simulate variable computational load per task
                double val = 0.0;
                int iterations = (i % 10 == 0) ? 10000 : 100;
                for (int j = 0; j < iterations; ++j) {
                    val += std::sin(j) * std::cos(j);
                }
                return val;
                }));
        }

        for (auto& f : futures) {
            benchmark::DoNotOptimize(f.get());
        }
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_WorkStealing_Imbalanced)->Arg(50000)->Unit(benchmark::kMillisecond);

// -------------------------------------------------------------
// Baseline Benchmark: std::async comparison
// -------------------------------------------------------------
static void BM_StdAsync_Baseline(benchmark::State& state) {
    const size_t num_tasks = state.range(0);

    for (auto _ : state) {
        std::vector<std::future<int>> futures;
        futures.reserve(num_tasks);

        for (size_t i = 0; i < num_tasks; ++i) {
            futures.push_back(std::async(std::launch::async, [](int x) { return x * x; }, static_cast<int>(i)));
        }

        for (auto& f : futures) {
            benchmark::DoNotOptimize(f.get());
        }
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_StdAsync_Baseline)->Arg(10000)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
