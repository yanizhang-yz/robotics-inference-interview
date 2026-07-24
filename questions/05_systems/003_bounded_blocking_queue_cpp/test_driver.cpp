#ifndef QUEST_HEADER
#define QUEST_HEADER "reference.hpp"
#endif

#include QUEST_HEADER

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

#define CHECK(condition)                                                       \
  do {                                                                         \
    if (!(condition)) {                                                        \
      throw std::runtime_error(std::string("CHECK failed at line ") +          \
                               std::to_string(__LINE__) + ": " #condition);   \
    }                                                                          \
  } while (false)

void test_fifo_single_thread() {
  BoundedBlockingQueue<int> queue(3);

  CHECK(queue.capacity() == 3);
  CHECK(queue.size() == 0);
  CHECK(queue.push(1));
  CHECK(queue.push(2));
  CHECK(queue.push(3));
  CHECK(queue.size() == 3);

  auto first = queue.pop();
  auto second = queue.pop();
  auto third = queue.pop();

  CHECK(first.has_value());
  CHECK(second.has_value());
  CHECK(third.has_value());
  CHECK(*first == 1);
  CHECK(*second == 2);
  CHECK(*third == 3);
  CHECK(queue.size() == 0);
}

void test_close_drains_existing_items() {
  BoundedBlockingQueue<int> queue(2);

  CHECK(queue.push(10));
  CHECK(queue.push(20));
  queue.close();

  CHECK(queue.closed());
  CHECK(!queue.push(30));

  auto first = queue.pop();
  auto second = queue.pop();
  auto stopped = queue.pop();

  CHECK(first.has_value());
  CHECK(second.has_value());
  CHECK(!stopped.has_value());
  CHECK(*first == 10);
  CHECK(*second == 20);
}

void test_supports_move_only_values() {
  BoundedBlockingQueue<std::unique_ptr<int>> queue(1);

  CHECK(queue.push(std::make_unique<int>(42)));

  auto item = queue.pop();

  CHECK(item.has_value());
  CHECK(*item != nullptr);
  CHECK(**item == 42);
}

void test_push_blocks_until_space_is_available() {
  BoundedBlockingQueue<int> queue(1);
  std::atomic<bool> push_finished = false;
  std::atomic<bool> push_accepted = false;

  CHECK(queue.push(1));

  std::thread producer([&]() {
    push_accepted = queue.push(2);
    push_finished = true;
  });

  std::this_thread::sleep_for(25ms);
  CHECK(!push_finished.load());

  auto first = queue.pop();
  CHECK(first.has_value());
  CHECK(*first == 1);

  for (int attempt = 0; attempt < 100 && !push_finished.load(); ++attempt) {
    std::this_thread::sleep_for(1ms);
  }

  CHECK(push_finished.load());
  CHECK(push_accepted.load());
  producer.join();

  auto second = queue.pop();
  CHECK(second.has_value());
  CHECK(*second == 2);
}

void test_multiple_producers_and_consumers() {
  constexpr int kProducers = 4;
  constexpr int kConsumers = 4;
  constexpr int kItemsPerProducer = 50;
  constexpr int kTotalItems = kProducers * kItemsPerProducer;

  BoundedBlockingQueue<int> queue(8);
  std::mutex consumed_mutex;
  std::atomic<bool> producer_failed = false;
  std::vector<int> consumed;
  std::vector<std::thread> producers;
  std::vector<std::thread> consumers;

  consumed.reserve(kTotalItems);

  for (int consumer = 0; consumer < kConsumers; ++consumer) {
    consumers.emplace_back([&]() {
      while (true) {
        auto item = queue.pop();
        if (!item.has_value()) {
          break;
        }
        std::lock_guard<std::mutex> lock(consumed_mutex);
        consumed.push_back(*item);
      }
    });
  }

  for (int producer = 0; producer < kProducers; ++producer) {
    producers.emplace_back([&, producer]() {
      for (int i = 0; i < kItemsPerProducer; ++i) {
        const int value = producer * kItemsPerProducer + i;
        if (!queue.push(value)) {
          producer_failed = true;
          return;
        }
      }
    });
  }

  for (auto& producer : producers) {
    producer.join();
  }

  queue.close();

  for (auto& consumer : consumers) {
    consumer.join();
  }

  CHECK(!producer_failed.load());
  std::sort(consumed.begin(), consumed.end());
  CHECK(consumed.size() == static_cast<std::size_t>(kTotalItems));
  for (int expected = 0; expected < kTotalItems; ++expected) {
    CHECK(consumed[static_cast<std::size_t>(expected)] == expected);
  }
}

int main(int argc, char** argv) {
  try {
    const std::string test_name = argc >= 2 ? argv[1] : "all";

    if (test_name == "fifo" || test_name == "all") {
      test_fifo_single_thread();
    }
    if (test_name == "close_drains" || test_name == "all") {
      test_close_drains_existing_items();
    }
    if (test_name == "move_only" || test_name == "all") {
      test_supports_move_only_values();
    }
    if (test_name == "blocks_when_full" || test_name == "all") {
      test_push_blocks_until_space_is_available();
    }
    if (test_name == "multi_producer_consumer" || test_name == "all") {
      test_multiple_producers_and_consumers();
    }
  } catch (const std::exception& exc) {
    std::cerr << exc.what() << '\n';
    return 1;
  }

  return 0;
}
