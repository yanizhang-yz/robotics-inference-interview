# Quest 06: C++ Ring Buffer

## Interview Prompt

Implement a fixed-capacity ring buffer for recent sensor samples. The buffer should
avoid dynamic allocation after construction, preserve FIFO behavior, and expose an
ASCII visualization of head and tail positions.

## Requirements

Implement:

```cpp
template <typename T>
class RingBuffer {
 public:
  explicit RingBuffer(std::size_t capacity);

  bool push(T item);
  std::optional<T> pop();
  std::size_t size() const;
  std::size_t capacity() const;
  bool empty() const;
  bool full() const;
  std::string visualize() const;
};
```

Behavior:

- `push` returns `false` when the buffer is full.
- `pop` returns `std::nullopt` when the buffer is empty.
- FIFO order is preserved across wraparound.
- The buffer stores move-only values.
- `visualize` shows occupied slots plus `H` and `T` markers.

## System Context

This sits in real-time-ish paths where recent values matter:

```text
camera frames / robot states / telemetry
-> ring buffer
-> timestamp alignment or policy observation builder
```

## Why It Exists

A ring buffer gives fixed memory, predictable push/pop cost, and a clean way to keep
recent samples without allocating on every control tick.

## Problem Solved

It prevents unbounded memory growth and avoids dynamic allocation in a hot path.

## Failure Modes

- Head/tail off-by-one bugs lose or duplicate samples.
- Full and empty states are confused.
- Wraparound corrupts FIFO order.
- Dynamic allocation sneaks into the control loop.

## Metrics

- push/pop latency
- dropped samples due to full buffer
- current size
- max observed size
- allocation count after construction

## Improvements After The Basic Version

- overwrite-oldest policy
- timestamp lookup
- lock-free SPSC ring buffer
- zero-copy views into stored samples
- visualizer with indices and timestamps

## Interview Follow-Ups

- How do you distinguish full from empty?
- Why is fixed capacity useful for robotics?
- What is the tradeoff between rejecting new samples and overwriting old samples?
- How does this become a lock-free SPSC queue?

## Player Task

```bash
cd "/Users/yanizhang/Documents/Inference engineer/robotics-inference-lab"
CPP_QUEST_IMPL=starter .venv/bin/python -m pytest -q quests/06-cpp-ring-buffer/tests/test_ring_buffer.py
```
