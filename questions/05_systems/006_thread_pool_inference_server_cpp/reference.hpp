#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

class ThreadPool {
 public:
  explicit ThreadPool(std::size_t num_workers) {
    if (num_workers == 0) {
      throw std::invalid_argument("num_workers must be > 0");
    }

    workers_.reserve(num_workers);
    for (std::size_t i = 0; i < num_workers; ++i) {
      workers_.emplace_back([this]() { worker_loop(); });
    }
  }

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  ~ThreadPool() {
    shutdown();
  }

  template <typename Fn>
  auto submit(Fn fn) -> std::future<std::invoke_result_t<Fn>> {
    using Result = std::invoke_result_t<Fn>;
    auto task =
        std::make_shared<std::packaged_task<Result()>>(std::move(fn));
    auto future = task->get_future();

    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (stopped_) {
        throw std::runtime_error("submit after shutdown");
      }
      tasks_.push([task]() { (*task)(); });
    }
    cv_.notify_one();
    return future;
  }

  void shutdown() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (stopped_) {
        return;
      }
      stopped_ = true;
    }
    cv_.notify_all();

    for (auto& worker : workers_) {
      if (worker.joinable()) {
        worker.join();
      }
    }
  }

 private:
  void worker_loop() {
    while (true) {
      std::function<void()> task;
      {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return stopped_ || !tasks_.empty(); });
        if (stopped_ && tasks_.empty()) {
          return;
        }
        task = std::move(tasks_.front());
        tasks_.pop();
      }
      task();
    }
  }

  std::vector<std::thread> workers_;
  std::queue<std::function<void()>> tasks_;
  std::mutex mutex_;
  std::condition_variable cv_;
  bool stopped_ = false;
};
