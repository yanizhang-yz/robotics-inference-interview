#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <future>
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
        max_wait_(max_wait) {}

  std::future<R> infer(T item) {
    return std::async(std::launch::async, [this, item = std::move(item)]() {
      return infer_batch_(std::vector<T>{item}).at(0);
    });
  }

  void shutdown() {}

 private:
  BatchFn infer_batch_;
  std::size_t max_batch_size_;
  std::chrono::milliseconds max_wait_;
};
