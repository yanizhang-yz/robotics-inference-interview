#ifndef QUEST_HEADER
#define QUEST_HEADER "reference.hpp"
#endif

#include QUEST_HEADER

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#define CHECK(condition)                                                       \
  do {                                                                         \
    if (!(condition)) {                                                        \
      throw std::runtime_error(std::string("CHECK failed at line ") +          \
                               std::to_string(__LINE__) + ": " #condition);   \
    }                                                                          \
  } while (false)

void test_owned_tensor_is_move_only_and_contiguous() {
  static_assert(!std::is_copy_constructible_v<OwnedTensor>);
  static_assert(std::is_move_constructible_v<OwnedTensor>);

  OwnedTensor tensor({1.0F, 2.0F, 3.0F});
  CHECK(tensor.size() == 3);
  CHECK(tensor.data() != nullptr);
  CHECK(tensor.values()[0] == 1.0F);
  CHECK(&tensor.values()[1] == tensor.data() + 1);

  OwnedTensor moved(std::move(tensor));
  CHECK(moved.size() == 3);
  CHECK(moved.values()[2] == 3.0F);
}

void test_dot_product_and_shape_validation() {
  const std::vector<float> lhs{1.0F, 2.0F, 3.0F};
  const std::vector<float> rhs{4.0F, 5.0F, 6.0F};
  CHECK(dot_product(lhs, rhs) == 32.0F);

  bool threw = false;
  try {
    dot_product(lhs, std::span<const float>(rhs.data(), 2));
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  CHECK(threw);
}

void test_normalize_in_place() {
  std::vector<float> values{2.0F, 3.0F, 5.0F};
  normalize_in_place(values);
  CHECK(std::fabs(values[0] - 0.2F) < 1.0e-5F);
  CHECK(std::fabs(values[1] - 0.3F) < 1.0e-5F);
  CHECK(std::fabs(values[2] - 0.5F) < 1.0e-5F);
}

void test_scaled_copy_does_not_alias_input() {
  std::vector<float> values{1.0F, 2.0F, 3.0F};
  OwnedTensor scaled = make_scaled_copy(values, 10.0F);

  CHECK(scaled.size() == 3);
  CHECK(scaled.values()[0] == 10.0F);
  CHECK(scaled.values()[2] == 30.0F);
  values[0] = 99.0F;
  CHECK(scaled.values()[0] == 10.0F);
}

int main(int argc, char** argv) {
  try {
    const std::string test_name = argc >= 2 ? argv[1] : "all";
    if (test_name == "ownership" || test_name == "all") {
      test_owned_tensor_is_move_only_and_contiguous();
    }
    if (test_name == "dot" || test_name == "all") {
      test_dot_product_and_shape_validation();
    }
    if (test_name == "normalize" || test_name == "all") {
      test_normalize_in_place();
    }
    if (test_name == "scaled_copy" || test_name == "all") {
      test_scaled_copy_does_not_alias_input();
    }
  } catch (const std::exception& exc) {
    std::cerr << exc.what() << '\n';
    return 1;
  }
  return 0;
}
