#include <iostream>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <numeric>
#include <random>

// A wrapper to allow move-only types (like std::packaged_task) in std::function
class TaskWrapper {
    struct ImplBase {
        virtual void call() = 0;
        virtual ~ImplBase() = default;
    };
    template <typename F>
    struct Impl : ImplBase {
        F f;
        Impl(F&& f) : f(std::forward<F>(f)) {}
        void call() override { f(); }
    };
    std::unique_ptr<ImplBase> impl;
public:
    template <typename F>
    TaskWrapper(F&& f) : impl(std::make_unique<Impl<F>>(std::forward<F>(f))) {}
    void operator()() { impl->call(); }
    TaskWrapper() = default;
    TaskWrapper(TaskWrapper&&) = default;
    TaskWrapper& operator=(TaskWrapper&&) = default;
};

class WorkStealingPool {
    struct LocalQueue {
        std::deque<TaskWrapper> queue;
        std::mutex mux;

        void push(TaskWrapper task) {
            std::lock_guard<std::mutex> lock(mux);
            queue.push_front(std::move(task));
        }

        bool try_pop(TaskWrapper& task) {
            std::lock_guard<std::mutex> lock(mux);
            if (queue.empty()) return false;
            task = std::move(queue.front());
            queue.pop_front();
            return true;
        }

        bool try_steal(TaskWrapper& task) {
            std::lock_guard<std::mutex> lock(mux);
            if (queue.empty()) return false;
            task = std::move(queue.back()); // Steal from the back to reduce contention
            queue.pop_back();
            return true;
        }
    };

    std::vector<std::thread> workers;
    std::vector<std::unique_ptr<LocalQueue>> local_queues;
    std::atomic<bool> stop{ false };
    std::atomic<size_t> total_tasks{ 0 };
    inline static thread_local int my_index = -1;

public:
    WorkStealingPool(size_t thread_count = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < thread_count; ++i) {
            local_queues.push_back(std::make_unique<LocalQueue>());
        }

        for (size_t i = 0; i < thread_count; ++i) {
            workers.emplace_back([this, i] {
                my_index = i;
                while (!stop) {
                    TaskWrapper task;
                    bool found = false;

                    // 1. Try local queue
                    if (local_queues[my_index]->try_pop(task)) {
                        found = true;
                    }
                    // 2. Try stealing from others
                    else {
                        for (size_t j = 0; j < local_queues.size(); ++j) {
                            size_t victim = (i + j + 1) % local_queues.size();
                            if (local_queues[victim]->try_steal(task)) {
                                found = true;
                                break;
                            }
                        }
                    }

                    if (found) {
                        task();
                        total_tasks--;
                    }
                    else {
                        std::this_thread::yield(); // Don't burn CPU if empty
                    }
                }
                });
        }
    }

    template <typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        using return_type = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();

        TaskWrapper wrapped_task([task]() { (*task)(); });

        total_tasks++;
        if (my_index != -1) {
            local_queues[my_index]->push(std::move(wrapped_task));
        }
        else {
            // Fallback for main thread: push to a random queue
            static thread_local std::mt19937 gen(std::hash<std::thread::id>{}(std::this_thread::get_id()));
            std::uniform_int_distribution<> dist(0, local_queues.size() - 1);
            local_queues[dist(gen)]->push(std::move(wrapped_task));
        }

        return res;
    }

    ~WorkStealingPool() {
        stop = true;
        for (auto& worker : workers) {
            if (worker.joinable()) worker.join();
        }
    }

    size_t get_pending_tasks() const { return total_tasks.load(); }
};

// --- Example Usage & Benchmarking logic ---

