#pragma once

#include <cstddef>
#include <optional>

template <typename T>
class SpscQueue {
 public:
  explicit SpscQueue(std::size_t capacity) : capacity_(capacity) {}

  bool try_push(T item) {
    (void)item;
    return false;
  }

  std::optional<T> try_pop() {
    return std::nullopt;
  }

  std::size_t capacity() const {
    return capacity_;
  }

  std::size_t size() const {
    return 0;
  }

  bool empty() const {
    return true;
  }

  bool full() const {
    return false;
  }

 private:
  std::size_t capacity_;
};
