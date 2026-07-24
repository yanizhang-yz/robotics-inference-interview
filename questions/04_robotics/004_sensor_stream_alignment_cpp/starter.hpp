#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

template <typename T>
struct Timed {
  std::int64_t timestamp_ns;
  T value;
};

struct Observation {
  std::int64_t timestamp_ns;
  int frame_id;
  double joint_position;
};

class SensorAligner {
 public:
  explicit SensorAligner(std::size_t max_samples_per_stream)
      : max_samples_per_stream_(max_samples_per_stream) {}

  void add_camera(Timed<int> frame) {
    (void)frame;
  }

  void add_joint(Timed<double> joint) {
    (void)joint;
  }

  std::optional<Observation> latest_aligned(std::int64_t max_skew_ns) const {
    (void)max_skew_ns;
    return std::nullopt;
  }

  std::size_t camera_size() const {
    return 0;
  }

  std::size_t joint_size() const {
    return 0;
  }

 private:
  std::size_t max_samples_per_stream_;
};
