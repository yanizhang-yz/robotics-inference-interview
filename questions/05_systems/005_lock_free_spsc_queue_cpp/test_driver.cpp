#ifndef QUEST_HEADER
#define QUEST_HEADER "reference.hpp"
#endif

#include QUEST_HEADER

#include <algorithm>
#include <atomic>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#define CHECK(condition)                                                       \
  do {                                                                         \
    if (!(condition)) {                                                        \
      throw std::runtime_error(std::string("CHECK failed at line ") +          \
                               std::to_string(__LINE__) + ": " #condition);   \
    }                                                                          \
  } while (false)

void test_single_thread_fifo_and_full() {
  SpscQueue<int> queue(2);
  CHECK(queue.empty());
  CHECK(queue.try_push(1));
  CHECK(queue.try_push(2));
  CHECK(queue.full());
  CHECK(!queue.try_push(3));
  CHECK(queue.try_pop().value() == 1);
  CHECK(queue.try_push(3));
  CHECK(queue.try_pop().value() == 2);
  CHECK(queue.try_pop().value() == 3);
  CHECK(!queue.try_pop().has_value());
}

void test_move_only_values() {
  SpscQueue<std::unique_ptr<int>> queue(1);
  CHECK(queue.try_push(std::make_unique<int>(42)));
  auto item = queue.try_pop();
  CHECK(item.has_value());
  CHECK(**item == 42);
}

void test_concurrent_single_producer_single_consumer() {
  constexpr int kItems = 10000;
  SpscQueue<int> queue(64);
  std::atomic<bool> producer_done = false;
  std::vector<int> consumed;
  consumed.reserve(kItems);

  std::thread producer([&]() {
    for (int value = 0; value < kItems; ++value) {
      while (!queue.try_push(value)) {
        std::this_thread::yield();
      }
    }
    producer_done = true;
  });

  std::thread consumer([&]() {
    while (!producer_done.load() || !queue.empty()) {
      auto item = queue.try_pop();
      if (item.has_value()) {
        consumed.push_back(*item);
      } else {
        std::this_thread::yield();
      }
    }
  });

  producer.join();
  consumer.join();

  CHECK(consumed.size() == static_cast<std::size_t>(kItems));
  for (int expected = 0; expected < kItems; ++expected) {
    CHECK(consumed[static_cast<std::size_t>(expected)] == expected);
  }
}

int main(int argc, char** argv) {
  try {
    const std::string test_name = argc >= 2 ? argv[1] : "all";
    if (test_name == "fifo" || test_name == "all") {
      test_single_thread_fifo_and_full();
    }
    if (test_name == "move_only" || test_name == "all") {
      test_move_only_values();
    }
    if (test_name == "concurrent" || test_name == "all") {
      test_concurrent_single_producer_single_consumer();
    }
  } catch (const std::exception& exc) {
    std::cerr << exc.what() << '\n';
    return 1;
  }
  return 0;
}
