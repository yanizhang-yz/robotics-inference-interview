# Quest 07: C++ Lock-Free SPSC Queue

## Interview Prompt

Implement a bounded lock-free single-producer/single-consumer queue for a sensor path.
One thread writes samples, one thread reads samples, and neither side should take a
mutex in the hot path.

## Requirements

Implement:

```cpp
template <typename T>
class SpscQueue {
 public:
  explicit SpscQueue(std::size_t capacity);

  bool try_push(T item);
  std::optional<T> try_pop();
  std::size_t capacity() const;
  std::size_t size() const;
  bool empty() const;
  bool full() const;
};
```

Behavior:

- exactly one producer thread calls `try_push`
- exactly one consumer thread calls `try_pop`
- `try_push` returns `false` when full
- `try_pop` returns `std::nullopt` when empty
- FIFO order is preserved
- move-only values are supported
- no mutex or condition variable is used

## System Context

This fits a simple sensor stream:

```text
camera capture thread
-> SPSC queue
-> preprocessing / alignment thread
-> policy inference
```

It also appears in logging, telemetry, and CPU/GPU staging paths where ownership is
simple: one writer, one reader.

## Why It Exists

When there is only one producer and one consumer, a mutex can be unnecessary overhead.
Atomic head/tail indices can coordinate the two threads with predictable memory use.

## Problem Solved

This reduces lock contention and teaches the ring-buffer foundation behind many
high-performance queues.

## Failure Modes

- using it with multiple producers breaks correctness
- wrong memory ordering can expose partially written data
- confusing full and empty drops or duplicates samples
- busy spinning without backoff can burn CPU

## Metrics

- successful pushes/sec
- successful pops/sec
- drops due to full queue
- queue depth
- producer/consumer lag

## Improvements After The Basic Version

- cache-line padding to reduce false sharing
- overwrite-oldest policy
- blocking wrapper with backoff
- timestamp-aware queue entries
- benchmark against a mutex queue

## Interview Follow-Ups

- Why is SPSC easier than MPMC?
- What does acquire/release ordering protect?
- Why do we keep one empty slot?
- When would a mutex queue be better?

## Player Task

```bash
cd "/Users/yanizhang/Documents/Inference engineer/robotics-inference-lab"
CPP_QUEST_IMPL=starter .venv/bin/python -m pytest -q quests/07-cpp-lock-free-spsc-queue/tests/test_spsc_queue.py
```
