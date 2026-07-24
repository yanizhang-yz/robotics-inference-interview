#pragma once

#include <cstddef>

class FixedBlockPool {
 public:
  FixedBlockPool(std::size_t block_size, std::size_t block_count)
      : block_size_(block_size), block_count_(block_count) {}

  void* acquire() {
    return nullptr;
  }

  void release(void* ptr) {
    (void)ptr;
  }

  bool owns(const void* ptr) const {
    (void)ptr;
    return false;
  }

  std::size_t available() const {
    return 0;
  }

  std::size_t capacity() const {
    return block_count_;
  }

  std::size_t block_size() const {
    return block_size_;
  }

 private:
  std::size_t block_size_;
  std::size_t block_count_;
};
