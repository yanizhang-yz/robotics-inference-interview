#ifndef QUEST_HEADER
#define QUEST_HEADER "reference.hpp"
#endif

#include QUEST_HEADER

#include <cmath>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#define CHECK(condition)                                                       \
  do {                                                                         \
    if (!(condition)) {                                                        \
      throw std::runtime_error(std::string("CHECK failed at line ") +          \
                               std::to_string(__LINE__) + ": " #condition);   \
    }                                                                          \
  } while (false)

void test_softmax_probabilities_sum_to_one() {
  const std::vector<float> logits{1.0F, 2.0F, 3.0F};
  const std::vector<float> probabilities = softmax(logits);
  const float total =
      std::accumulate(probabilities.begin(), probabilities.end(), 0.0F);
  CHECK(probabilities.size() == 3);
  CHECK(std::fabs(total - 1.0F) < 1.0e-5F);
  CHECK(probabilities[2] > probabilities[1]);
  CHECK(probabilities[1] > probabilities[0]);
}

void test_softmax_is_stable_for_large_logits() {
  const std::vector<float> logits{1000.0F, 1001.0F, 1002.0F};
  const std::vector<float> probabilities = softmax(logits);
  for (float probability : probabilities) {
    CHECK(std::isfinite(probability));
  }
}

void test_argmax_returns_first_max() {
  const std::vector<float> values{1.0F, 5.0F, 5.0F, 2.0F};
  CHECK(argmax(values) == 1);
}

void test_top_k_indices_are_sorted_by_value() {
  const std::vector<float> values{0.1F, 9.0F, 3.0F, 7.0F};
  const std::vector<std::size_t> top = top_k_indices(values, 3);
  CHECK(top.size() == 3);
  CHECK(top[0] == 1);
  CHECK(top[1] == 3);
  CHECK(top[2] == 2);
}

void test_invalid_inputs_throw() {
  bool threw = false;
  try {
    (void)softmax(std::span<const float>{});
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  CHECK(threw);

  threw = false;
  try {
    (void)top_k_indices(std::vector<float>{1.0F}, 0);
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  CHECK(threw);
}

int main(int argc, char** argv) {
  try {
    const std::string test_name = argc >= 2 ? argv[1] : "all";
    if (test_name == "softmax" || test_name == "all") {
      test_softmax_probabilities_sum_to_one();
    }
    if (test_name == "stable" || test_name == "all") {
      test_softmax_is_stable_for_large_logits();
    }
    if (test_name == "argmax" || test_name == "all") {
      test_argmax_returns_first_max();
    }
    if (test_name == "topk" || test_name == "all") {
      test_top_k_indices_are_sorted_by_value();
    }
    if (test_name == "invalid" || test_name == "all") {
      test_invalid_inputs_throw();
    }
  } catch (const std::exception& exc) {
    std::cerr << exc.what() << '\n';
    return 1;
  }
  return 0;
}
