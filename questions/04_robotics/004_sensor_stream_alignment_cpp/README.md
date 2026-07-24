# Quest 08: C++ Sensor Stream Alignment

## Interview Prompt

Implement a small sensor stream aligner for a robot policy server. Camera frames and
joint states arrive asynchronously with timestamps. The policy should receive a
coherent observation assembled from samples close enough in time.

## Input and Output Contract

The public interface is the declaration shown below. Inputs, return values,
blocking behavior, ownership rules, and shutdown behavior are part of the
contract and are exercised by `test_solution.py` plus `test_driver.cpp`.

```cpp
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
  explicit SensorAligner(std::size_t max_samples_per_stream);

  void add_camera(Timed<int> frame);
  void add_joint(Timed<double> joint);
  std::optional<Observation> latest_aligned(std::int64_t max_skew_ns) const;
  std::size_t camera_size() const;
  std::size_t joint_size() const;
};
```

## Requirements

Implement:

```cpp
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
  explicit SensorAligner(std::size_t max_samples_per_stream);

  void add_camera(Timed<int> frame);
  void add_joint(Timed<double> joint);
  std::optional<Observation> latest_aligned(std::int64_t max_skew_ns) const;
  std::size_t camera_size() const;
  std::size_t joint_size() const;
};
```

Behavior:

- store recent camera and joint samples
- evict oldest samples beyond capacity
- return the newest camera/joint pair whose timestamp skew is within the limit
- observation timestamp is the newer of the two sample timestamps
- return `std::nullopt` when no valid pair exists

## System Context

This sits before policy inference:

```text
camera stream + joint state stream
-> timestamp alignment
-> policy observation tensor
-> model inference
-> action queue
```

## Why It Exists

A robot policy needs a coherent observation. If the image is from 100 ms ago and the
joint state is current, the model may act on a physically impossible state.

## Problem Solved

This quest trains timestamp reasoning, bounded stream buffers, late data handling, and
the systems connection between sensors and inference.

## Failure Modes

- aligning stale camera frames with fresh joint state
- using wall-clock arrival time instead of sensor timestamp
- letting buffers grow forever
- silently using samples outside the control-loop tolerance

## Metrics

- alignment skew
- dropped samples
- stream lag
- empty alignment count
- observation age at policy inference time

## Improvements After The Basic Version

- interpolation
- per-stream latency estimates
- clock skew correction
- out-of-order sample handling
- multiple camera streams

## Interview Follow-Ups

- Why align by timestamp instead of arrival order?
- What happens if one sensor is delayed?
- Would you drop old samples or hold the control loop?
- How does alignment error affect policy behavior?

## Player Task

```bash
cd "/Users/yanizhang/Documents/Inference engineer/robotics-inference-lab"
CPP_QUEST_IMPL=starter .venv/bin/python -m pytest -q quests/08-cpp-sensor-stream-alignment/tests/test_sensor_stream_alignment.py
```

## Complexity Targets

Let `c` and `j` be the retained camera and joint sample counts, each at most the
configured capacity. Adding a sample and evicting the oldest sample is amortized
`O(1)`, size queries are `O(1)`, and `latest_aligned` is `O(c * j)` time with
`O(1)` auxiliary space. Retained storage is `O(c + j)`. The class performs no
internal synchronization, so concurrent callers must synchronize externally; the
bounds exclude deque block-allocation latency, while copies of the fixed-size
camera and joint sample values remain `O(1)`.

## Interview Follow-ups

1. Which invariant makes this implementation correct?
2. What fails first under overload or malformed input?
3. Which metric would reveal that failure in production?
4. What changes for a robot control loop versus an offline batch job?
