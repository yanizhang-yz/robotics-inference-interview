# Quest 05: C++ Ownership And Memory Layout

## Interview Prompt

Design a tiny tensor buffer API for inference preprocessing code. The API should own
memory safely, pass views without copying, support move semantics, and expose simple
operations over contiguous data.

## Requirements

Implement:

```cpp
class OwnedTensor {
 public:
  explicit OwnedTensor(std::vector<float> values);

  OwnedTensor(const OwnedTensor&) = delete;
  OwnedTensor& operator=(const OwnedTensor&) = delete;
  OwnedTensor(OwnedTensor&&) noexcept = default;
  OwnedTensor& operator=(OwnedTensor&&) noexcept = default;

  std::span<float> values();
  std::span<const float> values() const;
  std::size_t size() const;
  const float* data() const;
};

float dot_product(std::span<const float> lhs, std::span<const float> rhs);
void normalize_in_place(std::span<float> values);
OwnedTensor make_scaled_copy(std::span<const float> values, float scale);
```

Behavior:

- `OwnedTensor` owns a contiguous `std::vector<float>`.
- Copying is disabled to avoid accidental large tensor copies.
- Moving is allowed.
- Views are passed with `std::span` so functions do not take ownership.
- `dot_product` rejects mismatched shapes.
- `normalize_in_place` makes values sum to `1.0`.
- `make_scaled_copy` returns a new owned tensor without modifying the input.

## System Context

This lives in CPU preprocessing before model execution:

```text
request JSON / image bytes
-> decode / normalize / shape tensors
-> owned tensor buffers
-> model runtime
```

In robotics, the same pattern appears when assembling observations from camera frames,
joint state, gripper state, and previous actions.

## Why It Exists

Inference systems move a lot of data. Accidental copies increase latency and memory
pressure. Explicit ownership makes it clear who frees memory. Views let helpers read
or mutate buffers without taking ownership.

## Problem Solved

This quest builds muscle memory for RAII, move semantics, `std::vector`, `std::span`,
and copy avoidance.

## Failure Modes

- Returning views to destroyed memory causes use-after-free bugs.
- Copying large buffers in hot paths increases p95 latency.
- Mutating shared buffers accidentally corrupts requests.
- Shape mismatches silently produce wrong model inputs.

## Metrics

- number of tensor copies
- preprocessing latency
- allocation count
- peak memory
- bytes moved per request

## Improvements After The Basic Version

- add dtype and shape metadata
- add alignment guarantees
- add zero-copy image decode views
- add arena-backed tensor allocation
- add explicit CPU/GPU transfer timing

## Interview Follow-Ups

- Why delete copy construction?
- When would `std::shared_ptr` be worse than `std::unique_ptr`?
- Why use `std::span` instead of `const std::vector<float>&`?
- What does RAII buy us in exception paths?
- Where would accidental copies appear in an inference pipeline?

## Player Task

Fill in `starter.hpp`.

```bash
cd "/Users/yanizhang/Documents/Inference engineer/robotics-inference-lab"
CPP_QUEST_IMPL=starter .venv/bin/python -m pytest -q quests/05-cpp-ownership-memory-layout/tests/test_ownership_memory_layout.py
```

Reference:

```bash
.venv/bin/python -m pytest -q quests/05-cpp-ownership-memory-layout/tests/test_ownership_memory_layout.py
```
