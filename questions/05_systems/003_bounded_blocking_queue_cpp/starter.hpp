#pragma once

#include <cstddef>
#include <optional>

template <typename T>
class BoundedBlockingQueue {
 public:
  explicit BoundedBlockingQueue(std::size_t capacity) : capacity_(capacity) {}

  BoundedBlockingQueue(const BoundedBlockingQueue&) = delete;
  BoundedBlockingQueue& operator=(const BoundedBlockingQueue&) = delete;

  bool push(T item) {
    (void)item;
    return false;
  }

  std::optional<T> pop() {
    return std::nullopt;
  }

  void close() {
    closed_ = true;
  }

  std::size_t size() const {
    return 0;
  }

  std::size_t capacity() const {
    return capacity_;
  }

  bool closed() const {
    return closed_;
  }

 private:
  std::size_t capacity_;
  bool closed_ = false;
};
