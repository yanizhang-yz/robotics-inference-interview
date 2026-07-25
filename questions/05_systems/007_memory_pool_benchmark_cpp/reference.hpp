#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

class FixedBlockPool {
 public:
  FixedBlockPool(std::size_t block_size, std::size_t block_count)
      : block_size_(round_up(block_size)),
        requested_block_size_(block_size),
        block_count_(block_count),
        storage_(units_for(block_size_) * block_count),
        units_per_block_(units_for(block_size_)) {
    if (block_size == 0 || block_count == 0) {
      throw std::invalid_argument("block_size and block_count must be > 0");
    }
    free_indices_.reserve(block_count_);
    for (std::size_t i = 0; i < block_count_; ++i) {
      free_indices_.push_back(block_count_ - 1 - i);
    }
  }

  void* acquire() {
    if (free_indices_.empty()) {
      return nullptr;
    }
    const std::size_t index = free_indices_.back();
    free_indices_.pop_back();
    return block_ptr(index);
  }

  void release(void* ptr) {
    if (!owns(ptr)) {
      throw std::invalid_argument("pointer does not belong to pool");
    }
    const std::size_t index = block_index(ptr);
    free_indices_.push_back(index);
  }

  bool owns(const void* ptr) const {
    if (ptr == nullptr || storage_.empty()) {
      return false;
    }
    const auto* begin = reinterpret_cast<const std::byte*>(storage_.data());
    const auto* end = begin + storage_.size() * sizeof(std::max_align_t);
    const auto* current = reinterpret_cast<const std::byte*>(ptr);
    if (current < begin || current >= end) {
      return false;
    }
    const std::size_t offset = static_cast<std::size_t>(current - begin);
    return offset % block_size_ == 0;
  }

  std::size_t available() const {
    return free_indices_.size();
  }

  std::size_t capacity() const {
    return block_count_;
  }

  std::size_t block_size() const {
    return requested_block_size_;
  }

 private:
  static std::size_t round_up(std::size_t size) {
    const std::size_t align = alignof(std::max_align_t);
    return ((size + align - 1) / align) * align;
  }

  static std::size_t units_for(std::size_t bytes) {
    const std::size_t unit = sizeof(std::max_align_t);
    return (bytes + unit - 1) / unit;
  }

  void* block_ptr(std::size_t index) {
    return storage_.data() + index * units_per_block_;
  }

  std::size_t block_index(const void* ptr) const {
    const auto* begin = reinterpret_cast<const std::byte*>(storage_.data());
    const auto* current = reinterpret_cast<const std::byte*>(ptr);
    const std::size_t offset = static_cast<std::size_t>(current - begin);
    return offset / block_size_;
  }

  std::size_t block_size_;
  std::size_t requested_block_size_;
  std::size_t block_count_;
  std::vector<std::max_align_t> storage_;
  std::size_t units_per_block_;
  std::vector<std::size_t> free_indices_;
};
