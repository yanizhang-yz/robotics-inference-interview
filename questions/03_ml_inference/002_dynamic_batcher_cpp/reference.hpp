#pragma once

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

template <typename T, typename R>
class DynamicBatcher {
 public:
  using BatchFn = std::function<std::vector<R>(const std::vector<T>&)>;

  DynamicBatcher(BatchFn infer_batch,
                 std::size_t max_batch_size,
                 std::chrono::milliseconds max_wait)
      : infer_batch_(std::move(infer_batch)),
        max_batch_size_(max_batch_size),
        max_wait_(max_wait),
        worker_([this]() { worker_loop(); }) {
    if (max_batch_size == 0) {
      throw std::invalid_argument("max_batch_size must be > 0");
    }
  }

  DynamicBatcher(const DynamicBatcher&) = delete;
  DynamicBatcher& operator=(const DynamicBatcher&) = delete;

  ~DynamicBatcher() {
    shutdown();
  }

  std::future<R> infer(T item) {
    std::promise<R> promise;
    auto future = promise.get_future();
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (closed_) {
        throw std::runtime_error("infer after shutdown");
      }
      queue_.push_back(QueueItem{std::move(item), std::move(promise)});
    }
    cv_.notify_one();
    return future;
  }

  void shutdown() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (closed_) {
        return;
      }
      closed_ = true;
    }
    cv_.notify_all();
    if (worker_.joinable()) {
      worker_.join();
    }
  }

 private:
  struct QueueItem {
    T item;
    std::promise<R> promise;
  };

  void worker_loop() {
    while (true) {
      std::vector<QueueItem> batch = collect_batch();
      if (batch.empty()) {
        return;
      }
      run_batch(batch);
    }
  }

  std::vector<QueueItem> collect_batch() {
    std::vector<QueueItem> batch;
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]() { return closed_ || !queue_.empty(); });

    if (queue_.empty() && closed_) {
      return batch;
    }

    const auto deadline = std::chrono::steady_clock::now() + max_wait_;
    while (batch.size() < max_batch_size_) {
      if (!queue_.empty()) {
        batch.push_back(std::move(queue_.front()));
        queue_.pop_front();
        continue;
      }
      if (closed_) {
        break;
      }
      if (cv_.wait_until(lock, deadline) == std::cv_status::timeout) {
        break;
      }
    }
    return batch;
  }

  void run_batch(std::vector<QueueItem>& batch) {
    std::vector<T> inputs;
    inputs.reserve(batch.size());
    for (auto& request : batch) {
      inputs.push_back(std::move(request.item));
    }

    try {
      std::vector<R> outputs = infer_batch_(inputs);
      if (outputs.size() != batch.size()) {
        throw std::runtime_error("infer_batch returned wrong output count");
      }
      for (std::size_t i = 0; i < batch.size(); ++i) {
        batch[i].promise.set_value(std::move(outputs[i]));
      }
    } catch (...) {
      std::exception_ptr error = std::current_exception();
      for (auto& request : batch) {
        request.promise.set_exception(error);
      }
    }
  }

  BatchFn infer_batch_;
  std::size_t max_batch_size_;
  std::chrono::milliseconds max_wait_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::deque<QueueItem> queue_;
  bool closed_ = false;
  std::thread worker_;
};
