#pragma once

#include <cstddef>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

template <typename T>
class RingBuffer {
 public:
  explicit RingBuffer(std::size_t capacity)
      : slots_(capacity), capacity_(capacity) {
    if (capacity == 0) {
      throw std::invalid_argument("capacity must be > 0");
    }
  }

  bool push(T item) {
    if (full()) {
      return false;
    }
    slots_[tail_] = std::move(item);
    tail_ = (tail_ + 1) % capacity_;
    ++size_;
    return true;
  }

  std::optional<T> pop() {
    if (empty()) {
      return std::nullopt;
    }
    std::optional<T> item = std::move(slots_[head_]);
    slots_[head_].reset();
    head_ = (head_ + 1) % capacity_;
    --size_;
    return item;
  }

  std::size_t size() const {
    return size_;
  }

  std::size_t capacity() const {
    return capacity_;
  }

  bool empty() const {
    return size_ == 0;
  }

  bool full() const {
    return size_ == capacity_;
  }

  std::string visualize() const {
    std::ostringstream out;
    out << "capacity=" << capacity_ << " size=" << size_ << '\n';
    for (std::size_t i = 0; i < capacity_; ++i) {
      out << '[' << (slots_[i].has_value() ? 'X' : '_') << ']';
    }
    out << '\n';
    for (std::size_t i = 0; i < capacity_; ++i) {
      if (i == head_ && i == tail_) {
        out << "HT ";
      } else if (i == head_) {
        out << "H  ";
      } else if (i == tail_) {
        out << "T  ";
      } else {
        out << "   ";
      }
    }
    return out.str();
  }

 private:
  std::vector<std::optional<T>> slots_;
  std::size_t capacity_;
  std::size_t head_ = 0;
  std::size_t tail_ = 0;
  std::size_t size_ = 0;
};
