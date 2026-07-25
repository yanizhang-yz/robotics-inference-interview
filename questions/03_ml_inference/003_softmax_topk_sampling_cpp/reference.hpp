#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <span>
#include <stdexcept>
#include <vector>

std::vector<float> softmax(std::span<const float> logits) {
  if (logits.empty()) {
    throw std::invalid_argument("softmax requires non-empty logits");
  }

  const float max_logit = *std::max_element(logits.begin(), logits.end());
  std::vector<float> probabilities;
  probabilities.reserve(logits.size());

  float total = 0.0F;
  for (float logit : logits) {
    const float value = std::exp(logit - max_logit);
    probabilities.push_back(value);
    total += value;
  }

  for (float& probability : probabilities) {
    probability /= total;
  }
  return probabilities;
}

std::size_t argmax(std::span<const float> values) {
  if (values.empty()) {
    throw std::invalid_argument("argmax requires non-empty values");
  }

  std::size_t best = 0;
  for (std::size_t i = 1; i < values.size(); ++i) {
    if (values[i] > values[best]) {
      best = i;
    }
  }
  return best;
}

std::vector<std::size_t> top_k_indices(std::span<const float> values,
                                       std::size_t k) {
  if (values.empty()) {
    throw std::invalid_argument("top_k_indices requires non-empty values");
  }
  if (k == 0 || k > values.size()) {
    throw std::invalid_argument("k must be in [1, values.size()]");
  }

  std::vector<std::size_t> indices(values.size());
  std::iota(indices.begin(), indices.end(), 0);
  std::partial_sort(
      indices.begin(),
      indices.begin() + static_cast<std::ptrdiff_t>(k),
      indices.end(),
      [&](std::size_t lhs, std::size_t rhs) {
        if (values[lhs] == values[rhs]) {
          return lhs < rhs;
        }
        return values[lhs] > values[rhs];
      });
  indices.resize(k);
  return indices;
}
