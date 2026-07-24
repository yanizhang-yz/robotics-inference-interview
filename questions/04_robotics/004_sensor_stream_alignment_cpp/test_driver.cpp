#ifndef QUEST_HEADER
#define QUEST_HEADER "reference.hpp"
#endif

#include QUEST_HEADER

#include <iostream>
#include <stdexcept>
#include <string>

#define CHECK(condition)                                                       \
  do {                                                                         \
    if (!(condition)) {                                                        \
      throw std::runtime_error(std::string("CHECK failed at line ") +          \
                               std::to_string(__LINE__) + ": " #condition);   \
    }                                                                          \
  } while (false)

void test_aligns_latest_pair_within_skew() {
  SensorAligner aligner(4);
  aligner.add_camera({100, 1});
  aligner.add_camera({200, 2});
  aligner.add_joint({205, 0.75});

  auto observation = aligner.latest_aligned(10);
  CHECK(observation.has_value());
  CHECK(observation->timestamp_ns == 205);
  CHECK(observation->frame_id == 2);
  CHECK(observation->joint_position == 0.75);
}

void test_returns_null_when_no_pair_is_close_enough() {
  SensorAligner aligner(4);
  aligner.add_camera({100, 1});
  aligner.add_joint({500, 0.5});

  CHECK(!aligner.latest_aligned(50).has_value());
}

void test_capacity_evicts_oldest_samples() {
  SensorAligner aligner(2);
  aligner.add_camera({100, 1});
  aligner.add_camera({200, 2});
  aligner.add_camera({300, 3});
  aligner.add_joint({300, 1.5});

  CHECK(aligner.camera_size() == 2);
  auto observation = aligner.latest_aligned(0);
  CHECK(observation.has_value());
  CHECK(observation->frame_id == 3);
}

void test_uses_newest_valid_pair() {
  SensorAligner aligner(8);
  aligner.add_camera({100, 1});
  aligner.add_joint({102, 0.1});
  aligner.add_camera({300, 3});
  aligner.add_joint({301, 0.3});

  auto observation = aligner.latest_aligned(5);
  CHECK(observation.has_value());
  CHECK(observation->timestamp_ns == 301);
  CHECK(observation->frame_id == 3);
  CHECK(observation->joint_position == 0.3);
}

int main(int argc, char** argv) {
  try {
    const std::string test_name = argc >= 2 ? argv[1] : "all";
    if (test_name == "align" || test_name == "all") {
      test_aligns_latest_pair_within_skew();
    }
    if (test_name == "no_pair" || test_name == "all") {
      test_returns_null_when_no_pair_is_close_enough();
    }
    if (test_name == "capacity" || test_name == "all") {
      test_capacity_evicts_oldest_samples();
    }
    if (test_name == "newest" || test_name == "all") {
      test_uses_newest_valid_pair();
    }
  } catch (const std::exception& exc) {
    std::cerr << exc.what() << '\n';
    return 1;
  }
  return 0;
}
