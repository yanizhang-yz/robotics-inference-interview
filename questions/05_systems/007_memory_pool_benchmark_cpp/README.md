# Quest 11: C++ Memory Pool And Allocation Benchmark

## Interview Prompt

Implement a fixed-size memory block pool for a latency-sensitive inference path. The
pool should preallocate memory, hand out fixed-size blocks, reuse released blocks, and
fail predictably when exhausted.

## Input and Output Contract

The public interface is the declaration shown below. Inputs, return values,
blocking behavior, ownership rules, and shutdown behavior are part of the
contract and are exercised by `test_solution.py` plus `test_driver.cpp`.

```cpp
class FixedBlockPool {
 public:
  FixedBlockPool(std::size_t block_size, std::size_t block_count);

  void* acquire();
  void release(void* ptr);
  bool owns(const void* ptr) const;
  std::size_t available() const;
  std::size_t capacity() const;
  std::size_t block_size() const;
};
```

## Requirements

Implement:

```cpp
class FixedBlockPool {
 public:
  FixedBlockPool(std::size_t block_size, std::size_t block_count);

  void* acquire();
  void release(void* ptr);
  bool owns(const void* ptr) const;
  std::size_t available() const;
  std::size_t capacity() const;
  std::size_t block_size() const;
};
```

Behavior:

- allocate all storage in the constructor
- `acquire` returns `nullptr` when exhausted
- `release` returns a block to the pool
- released blocks are reused
- invalid releases throw
- no per-block heap allocation in `acquire`

## System Context

This sits in hot request paths:

```text
request preprocessing
-> temporary buffers
-> memory pool
-> model runtime
```

In robotics, bounded allocation is important because unpredictable allocation latency
can show up as control-loop jitter.

## Why It Exists

Heap allocation can be slow, fragmented, and unpredictable. A pool trades flexibility
for predictable allocation behavior.

## Problem Solved

This quest trains dynamic allocation awareness, ownership boundaries, reuse, and
real-time-ish memory discipline.

## Failure Modes

- returning the same block twice
- accepting pointers not owned by the pool
- use-after-release
- hidden heap allocation in the hot path
- block alignment mistakes

## Metrics

- allocation latency
- release latency
- pool exhaustion count
- peak blocks in use
- heap allocations per request

## Improvements After The Basic Version

- typed object pool
- RAII handle that releases automatically
- thread-safe pool
- per-thread pool
- allocation benchmark comparing pool vs `new`

## Interview Follow-Ups

- Why preallocate?
- What are the tradeoffs of fixed-size blocks?
- How would you make release exception-safe?
- Where would memory pooling help an inference server?

## Player Task

```bash
cd "/Users/yanizhang/Documents/Inference engineer/robotics-inference-lab"
CPP_QUEST_IMPL=starter .venv/bin/python -m pytest -q quests/11-cpp-memory-pool-benchmark/tests/test_memory_pool.py
```

## Complexity Targets

For requested block size `S` and block count `B`, construction initializes
`O(B * round_up(S))` bytes of block storage plus `O(B)` free-list entries and
retains `O(B * round_up(S) + B)` storage. `acquire`, `release`, `owns`, and each
size query are `O(1)`. Valid acquire/release cycles perform no heap allocation after
construction. The pool has no internal synchronization, so concurrent callers must
synchronize externally; the bounds exclude constructor allocator latency and cache
effects.

## Interview Follow-ups

1. Which invariant makes this implementation correct?
2. What fails first under overload or malformed input?
3. Which metric would reveal that failure in production?
4. What changes for a robot control loop versus an offline batch job?
