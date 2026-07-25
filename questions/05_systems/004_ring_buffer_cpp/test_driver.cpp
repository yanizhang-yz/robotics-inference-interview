#ifndef QUEST_HEADER
#define QUEST_HEADER "reference.hpp"
#endif

#include QUEST_HEADER

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#define CHECK(condition)                                                       \
  do {                                                                         \
    if (!(condition)) {                                                        \
      throw std::runtime_error(std::string("CHECK failed at line ") +          \
                               std::to_string(__LINE__) + ": " #condition);   \
    }                                                                          \
  } while (false)

void test_fifo_and_full_state() {
  RingBuffer<int> buffer(3);
  CHECK(buffer.empty());
  CHECK(buffer.push(1));
  CHECK(buffer.push(2));
  CHECK(buffer.push(3));
  CHECK(buffer.full());
  CHECK(!buffer.push(4));
  CHECK(buffer.pop().value() == 1);
  CHECK(buffer.pop().value() == 2);
  CHECK(buffer.pop().value() == 3);
  CHECK(!buffer.pop().has_value());
}

void test_wraparound_preserves_order() {
  RingBuffer<int> buffer(3);
  CHECK(buffer.push(1));
  CHECK(buffer.push(2));
  CHECK(buffer.pop().value() == 1);
  CHECK(buffer.push(3));
  CHECK(buffer.push(4));
  CHECK(buffer.full());
  CHECK(buffer.pop().value() == 2);
  CHECK(buffer.pop().value() == 3);
  CHECK(buffer.pop().value() == 4);
}

void test_move_only_values() {
  RingBuffer<std::unique_ptr<int>> buffer(2);
  CHECK(buffer.push(std::make_unique<int>(7)));
  auto item = buffer.pop();
  CHECK(item.has_value());
  CHECK(**item == 7);
}

void test_visualizer_shows_state() {
  RingBuffer<int> buffer(4);
  CHECK(buffer.push(10));
  CHECK(buffer.push(20));
  const std::string view = buffer.visualize();
  CHECK(view.find("capacity=4") != std::string::npos);
  CHECK(view.find("size=2") != std::string::npos);
  CHECK(view.find("H") != std::string::npos);
  CHECK(view.find("T") != std::string::npos);
  CHECK(view.find("[X]") != std::string::npos);
}

int main(int argc, char** argv) {
  try {
    const std::string test_name = argc >= 2 ? argv[1] : "all";
    if (test_name == "fifo" || test_name == "all") {
      test_fifo_and_full_state();
    }
    if (test_name == "wrap" || test_name == "all") {
      test_wraparound_preserves_order();
    }
    if (test_name == "move_only" || test_name == "all") {
      test_move_only_values();
    }
    if (test_name == "visualizer" || test_name == "all") {
      test_visualizer_shows_state();
    }
  } catch (const std::exception& exc) {
    std::cerr << exc.what() << '\n';
    return 1;
  }
  return 0;
}
