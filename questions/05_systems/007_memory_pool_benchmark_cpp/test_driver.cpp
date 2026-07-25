#ifndef QUEST_HEADER
#define QUEST_HEADER "reference.hpp"
#endif

#include QUEST_HEADER

#include <iostream>
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

void test_acquire_until_exhausted() {
  FixedBlockPool pool(64, 2);
  void* first = pool.acquire();
  void* second = pool.acquire();
  void* third = pool.acquire();

  CHECK(first != nullptr);
  CHECK(second != nullptr);
  CHECK(first != second);
  CHECK(third == nullptr);
  CHECK(pool.available() == 0);
}

void test_release_reuses_block() {
  FixedBlockPool pool(32, 1);
  void* first = pool.acquire();
  CHECK(first != nullptr);
  pool.release(first);
  CHECK(pool.available() == 1);
  void* second = pool.acquire();
  CHECK(second == first);
}

void test_owns_and_invalid_release() {
  FixedBlockPool pool(32, 1);
  int stack_value = 5;
  CHECK(!pool.owns(&stack_value));

  bool threw = false;
  try {
    pool.release(&stack_value);
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  CHECK(threw);
}

void test_metadata() {
  FixedBlockPool pool(128, 3);
  CHECK(pool.block_size() == 128);
  CHECK(pool.capacity() == 3);
  CHECK(pool.available() == 3);
}

int main(int argc, char** argv) {
  try {
    const std::string test_name = argc >= 2 ? argv[1] : "all";
    if (test_name == "exhausted" || test_name == "all") {
      test_acquire_until_exhausted();
    }
    if (test_name == "reuse" || test_name == "all") {
      test_release_reuses_block();
    }
    if (test_name == "owns" || test_name == "all") {
      test_owns_and_invalid_release();
    }
    if (test_name == "metadata" || test_name == "all") {
      test_metadata();
    }
  } catch (const std::exception& exc) {
    std::cerr << exc.what() << '\n';
    return 1;
  }
  return 0;
}
