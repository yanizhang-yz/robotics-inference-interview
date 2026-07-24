#ifndef QUEST_HEADER
#define QUEST_HEADER "reference.hpp"
#endif

#include QUEST_HEADER

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
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

void test_batches_concurrent_requests() {
  std::mutex batches_mutex;
  std::vector<std::vector<int>> seen_batches;

  DynamicBatcher<int, int> batcher(
      [&](const std::vector<int>& inputs) {
        std::lock_guard<std::mutex> lock(batches_mutex);
        seen_batches.push_back(inputs);
        std::vector<int> outputs;
        for (int value : inputs) {
          outputs.push_back(value * 10);
        }
        return outputs;
      },
      3,
      50ms);

  std::vector<std::future<int>> futures;
  for (int i = 0; i < 7; ++i) {
    futures.push_back(batcher.infer(i));
  }

  for (int i = 0; i < 7; ++i) {
    CHECK(futures[static_cast<std::size_t>(i)].get() == i * 10);
  }
  batcher.shutdown();

  bool saw_batch_larger_than_one = false;
  for (const auto& batch : seen_batches) {
    CHECK(batch.size() <= 3);
    if (batch.size() > 1) {
      saw_batch_larger_than_one = true;
    }
  }
  CHECK(saw_batch_larger_than_one);
}

void test_propagates_batch_errors() {
  DynamicBatcher<int, int> batcher(
      [](const std::vector<int>& inputs) {
        (void)inputs;
        throw std::runtime_error("model failed");
        return std::vector<int>{};
      },
      2,
      5ms);

  auto future = batcher.infer(1);
  bool threw = false;
  try {
    (void)future.get();
  } catch (const std::runtime_error& exc) {
    threw = std::string(exc.what()).find("model failed") != std::string::npos;
  }
  CHECK(threw);
  batcher.shutdown();
}

void test_shutdown_rejects_new_requests() {
  DynamicBatcher<int, int> batcher(
      [](const std::vector<int>& inputs) {
        std::vector<int> outputs;
        for (int value : inputs) {
          outputs.push_back(value);
        }
        return outputs;
      },
      2,
      5ms);

  batcher.shutdown();
  bool threw = false;
  try {
    (void)batcher.infer(1);
  } catch (const std::runtime_error&) {
    threw = true;
  }
  CHECK(threw);
}

int main(int argc, char** argv) {
  try {
    const std::string test_name = argc >= 2 ? argv[1] : "all";
    if (test_name == "batch" || test_name == "all") {
      test_batches_concurrent_requests();
    }
    if (test_name == "errors" || test_name == "all") {
      test_propagates_batch_errors();
    }
    if (test_name == "shutdown" || test_name == "all") {
      test_shutdown_rejects_new_requests();
    }
  } catch (const std::exception& exc) {
    std::cerr << exc.what() << '\n';
    return 1;
  }
  return 0;
}
