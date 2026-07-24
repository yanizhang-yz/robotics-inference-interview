#pragma once

#include <cstddef>
#include <span>
#include <vector>

std::vector<float> softmax(std::span<const float> logits) {
  (void)logits;
  return {};
}

std::size_t argmax(std::span<const float> values) {
  (void)values;
  return 0;
}

std::vector<std::size_t> top_k_indices(std::span<const float> values,
                                       std::size_t k) {
  (void)values;
  (void)k;
  return {};
}
