#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <optional>
#include <stdexcept>

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
      : max_samples_per_stream_(max_samples_per_stream) {
    if (max_samples_per_stream == 0) {
      throw std::invalid_argument("max_samples_per_stream must be > 0");
    }
  }

  void add_camera(Timed<int> frame) {
    cameras_.push_back(frame);
    trim(cameras_);
  }

  void add_joint(Timed<double> joint) {
    joints_.push_back(joint);
    trim(joints_);
  }

  std::optional<Observation> latest_aligned(std::int64_t max_skew_ns) const {
    std::optional<Observation> best;

    for (auto camera_it = cameras_.rbegin(); camera_it != cameras_.rend();
         ++camera_it) {
      for (auto joint_it = joints_.rbegin(); joint_it != joints_.rend();
           ++joint_it) {
        const std::int64_t skew =
            std::llabs(camera_it->timestamp_ns - joint_it->timestamp_ns);
        if (skew > max_skew_ns) {
          continue;
        }

        Observation candidate{
            std::max(camera_it->timestamp_ns, joint_it->timestamp_ns),
            camera_it->value,
            joint_it->value,
        };
        if (!best.has_value() || candidate.timestamp_ns > best->timestamp_ns) {
          best = candidate;
        }
      }
    }

    return best;
  }

  std::size_t camera_size() const {
    return cameras_.size();
  }

  std::size_t joint_size() const {
    return joints_.size();
  }

 private:
  template <typename T>
  void trim(std::deque<T>& samples) {
    while (samples.size() > max_samples_per_stream_) {
      samples.pop_front();
    }
  }

  std::size_t max_samples_per_stream_;
  std::deque<Timed<int>> cameras_;
  std::deque<Timed<double>> joints_;
};
