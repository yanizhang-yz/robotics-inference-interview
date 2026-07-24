# Quest 07: C++ Lock-Free SPSC Queue

## Interview Prompt

Implement a bounded lock-free single-producer/single-consumer queue for a sensor path.
One thread writes samples, one thread reads samples, and neither side should take a
mutex in the hot path.

## Input and Output Contract

The public interface is the declaration shown below. Inputs, return values,
blocking behavior, ownership rules, and shutdown behavior are part of the
contract and are exercised by `test_solution.py` plus `test_driver.cpp`.

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

## Complexity Targets

For capacity `C`, construction is `O(C)` time and retained space. Each `try_push`,
`try_pop`, and state query is `O(1)` and non-blocking, with no queue allocation
after construction. These guarantees require exactly one producer and one consumer.
The bounds exclude cache-coherence and atomic memory-ordering latency plus element
move/destruction costs; no mutex, condition-variable, or allocator cost is hidden in
the queue operations themselves.

## Interview Follow-ups

1. Which invariant makes this implementation correct?
2. What fails first under overload or malformed input?
3. Which metric would reveal that failure in production?
4. What changes for a robot control loop versus an offline batch job?
