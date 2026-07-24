#ifndef QUEST_HEADER
#define QUEST_HEADER "reference.hpp"
#endif

#include QUEST_HEADER

#include <atomic>
#include <chrono>
#include <iostream>
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

void test_returns_future_results() {
  ThreadPool pool(2);
  auto a = pool.submit([]() { return 10; });
  auto b = pool.submit([]() { return 32; });
  CHECK(a.get() == 10);
  CHECK(b.get() == 32);
  pool.shutdown();
}

void test_runs_tasks_concurrently() {
  ThreadPool pool(4);
  std::atomic<int> active = 0;
  std::atomic<int> max_active = 0;
  std::vector<std::future<int>> futures;

  for (int i = 0; i < 4; ++i) {
    futures.push_back(pool.submit([&]() {
      const int now = ++active;
      int previous = max_active.load();
      while (now > previous &&
             !max_active.compare_exchange_weak(previous, now)) {
      }
      std::this_thread::sleep_for(30ms);
      --active;
      return 1;
    }));
  }

  int total = 0;
  for (auto& future : futures) {
    total += future.get();
  }
  CHECK(total == 4);
  CHECK(max_active.load() > 1);
  pool.shutdown();
}

void test_shutdown_rejects_new_tasks() {
  ThreadPool pool(1);
  auto future = pool.submit([]() { return 7; });
  CHECK(future.get() == 7);
  pool.shutdown();

  bool threw = false;
  try {
    (void)pool.submit([]() { return 1; });
  } catch (const std::runtime_error&) {
    threw = true;
  }
  CHECK(threw);
}

int main(int argc, char** argv) {
  try {
    const std::string test_name = argc >= 2 ? argv[1] : "all";
    if (test_name == "future" || test_name == "all") {
      test_returns_future_results();
    }
    if (test_name == "concurrent" || test_name == "all") {
      test_runs_tasks_concurrently();
    }
    if (test_name == "shutdown" || test_name == "all") {
      test_shutdown_rejects_new_tasks();
    }
  } catch (const std::exception& exc) {
    std::cerr << exc.what() << '\n';
    return 1;
  }
  return 0;
}
