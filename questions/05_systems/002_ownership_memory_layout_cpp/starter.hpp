#pragma once

#include <cstddef>
#include <span>
#include <vector>

class OwnedTensor {
 public:
  explicit OwnedTensor(std::vector<float> values) : values_(std::move(values)) {}

  OwnedTensor(const OwnedTensor&) = delete;
  OwnedTensor& operator=(const OwnedTensor&) = delete;
  OwnedTensor(OwnedTensor&&) noexcept = default;
  OwnedTensor& operator=(OwnedTensor&&) noexcept = default;

  std::span<float> values() {
    return {};
  }

  std::span<const float> values() const {
    return {};
  }

  std::size_t size() const {
    return 0;
  }

  const float* data() const {
    return nullptr;
  }

 private:
  std::vector<float> values_;
};

float dot_product(std::span<const float> lhs, std::span<const float> rhs) {
  (void)lhs;
  (void)rhs;
  return 0.0F;
}

void normalize_in_place(std::span<float> values) {
  (void)values;
}

OwnedTensor make_scaled_copy(std::span<const float> values, float scale) {
  (void)values;
  (void)scale;
  return OwnedTensor({});
}
