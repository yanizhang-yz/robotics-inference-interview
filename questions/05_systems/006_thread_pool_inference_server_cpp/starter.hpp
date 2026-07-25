#pragma once

#include <cstddef>
#include <future>
#include <type_traits>

class ThreadPool {
 public:
  explicit ThreadPool(std::size_t num_workers) : num_workers_(num_workers) {}

  template <typename Fn>
  auto submit(Fn fn) -> std::future<std::invoke_result_t<Fn>> {
    using Result = std::invoke_result_t<Fn>;
    std::promise<Result> promise;
    auto future = promise.get_future();
    if constexpr (std::is_void_v<Result>) {
      fn();
      promise.set_value();
    } else {
      promise.set_value(fn());
    }
    return future;
  }

  void shutdown() {}

 private:
  std::size_t num_workers_;
};
