#pragma once

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <utility>

template <typename T>
class BoundedBlockingQueue {
 public:
  explicit BoundedBlockingQueue(std::size_t capacity) : capacity_(capacity) {
    if (capacity == 0) {
      throw std::invalid_argument("capacity must be > 0");
    }
  }

  BoundedBlockingQueue(const BoundedBlockingQueue&) = delete;
  BoundedBlockingQueue& operator=(const BoundedBlockingQueue&) = delete;

  bool push(T item) {
    std::unique_lock<std::mutex> lock(mutex_);
    not_full_.wait(lock, [&]() {
      return closed_ || items_.size() < capacity_;
    });

    if (closed_) {
      return false;
    }

    items_.push_back(std::move(item));
    not_empty_.notify_one();
    return true;
  }

  std::optional<T> pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    not_empty_.wait(lock, [&]() {
      return closed_ || !items_.empty();
    });

    if (items_.empty()) {
      return std::nullopt;
    }

    T item = std::move(items_.front());
    items_.pop_front();
    not_full_.notify_one();
    return item;
  }

  void close() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      closed_ = true;
    }
    not_full_.notify_all();
    not_empty_.notify_all();
  }

  std::size_t size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return items_.size();
  }

  std::size_t capacity() const {
    return capacity_;
  }

  bool closed() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return closed_;
  }

 private:
  const std::size_t capacity_;
  mutable std::mutex mutex_;
  std::condition_variable not_empty_;
  std::condition_variable not_full_;
  std::deque<T> items_;
  bool closed_ = false;
};
