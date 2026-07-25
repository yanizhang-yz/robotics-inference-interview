#pragma once

#include <atomic>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

template <typename T>
class SpscQueue {
 public:
  explicit SpscQueue(std::size_t capacity)
      : slots_(capacity + 1), capacity_(capacity), slot_count_(capacity + 1) {
    if (capacity == 0) {
      throw std::invalid_argument("capacity must be > 0");
    }
  }

  bool try_push(T item) {
    const std::size_t tail = tail_.load(std::memory_order_relaxed);
    const std::size_t next_tail = increment(tail);
    if (next_tail == head_.load(std::memory_order_acquire)) {
      return false;
    }

    slots_[tail] = std::move(item);
    tail_.store(next_tail, std::memory_order_release);
    return true;
  }

  std::optional<T> try_pop() {
    const std::size_t head = head_.load(std::memory_order_relaxed);
    if (head == tail_.load(std::memory_order_acquire)) {
      return std::nullopt;
    }

    std::optional<T> item = std::move(slots_[head]);
    slots_[head].reset();
    head_.store(increment(head), std::memory_order_release);
    return item;
  }

  std::size_t capacity() const {
    return capacity_;
  }

  std::size_t size() const {
    const std::size_t head = head_.load(std::memory_order_acquire);
    const std::size_t tail = tail_.load(std::memory_order_acquire);
    if (tail >= head) {
      return tail - head;
    }
    return slot_count_ - head + tail;
  }

  bool empty() const {
    return head_.load(std::memory_order_acquire) ==
           tail_.load(std::memory_order_acquire);
  }

  bool full() const {
    const std::size_t tail = tail_.load(std::memory_order_acquire);
    return increment(tail) == head_.load(std::memory_order_acquire);
  }

 private:
  std::size_t increment(std::size_t index) const {
    return (index + 1) % slot_count_;
  }

  std::vector<std::optional<T>> slots_;
  const std::size_t capacity_;
  const std::size_t slot_count_;
  std::atomic<std::size_t> head_{0};
  std::atomic<std::size_t> tail_{0};
};
