#pragma once

#include <cstddef>
#include <optional>
#include <string>

template <typename T>
class RingBuffer {
 public:
  explicit RingBuffer(std::size_t capacity) : capacity_(capacity) {}

  bool push(T item) {
    (void)item;
    return false;
  }

  std::optional<T> pop() {
    return std::nullopt;
  }

  std::size_t size() const {
    return 0;
  }

  std::size_t capacity() const {
    return capacity_;
  }

  bool empty() const {
    return true;
  }

  bool full() const {
    return false;
  }

  std::string visualize() const {
    return "";
  }

 private:
  std::size_t capacity_;
};
