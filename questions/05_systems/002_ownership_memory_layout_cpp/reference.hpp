#pragma once

#include <cmath>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

class OwnedTensor {
 public:
  explicit OwnedTensor(std::vector<float> values) : values_(std::move(values)) {}

  OwnedTensor(const OwnedTensor&) = delete;
  OwnedTensor& operator=(const OwnedTensor&) = delete;
  OwnedTensor(OwnedTensor&&) noexcept = default;
  OwnedTensor& operator=(OwnedTensor&&) noexcept = default;

  std::span<float> values() {
    return values_;
  }

  std::span<const float> values() const {
    return values_;
  }

  std::size_t size() const {
    return values_.size();
  }

  const float* data() const {
    return values_.data();
  }

 private:
  std::vector<float> values_;
};

static_assert(!std::is_copy_constructible_v<OwnedTensor>);
static_assert(std::is_move_constructible_v<OwnedTensor>);

float dot_product(std::span<const float> lhs, std::span<const float> rhs) {
  if (lhs.size() != rhs.size()) {
    throw std::invalid_argument("dot_product shape mismatch");
  }

  float result = 0.0F;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    result += lhs[i] * rhs[i];
  }
  return result;
}

void normalize_in_place(std::span<float> values) {
  float total = 0.0F;
  for (float value : values) {
    total += value;
  }
  if (std::fabs(total) < 1.0e-12F) {
    throw std::invalid_argument("cannot normalize zero-sum values");
  }
  for (float& value : values) {
    value /= total;
  }
}

OwnedTensor make_scaled_copy(std::span<const float> values, float scale) {
  std::vector<float> copied;
  copied.reserve(values.size());
  for (float value : values) {
    copied.push_back(value * scale);
  }
  return OwnedTensor(std::move(copied));
}
