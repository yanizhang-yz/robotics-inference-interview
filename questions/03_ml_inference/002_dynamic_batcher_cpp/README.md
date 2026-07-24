# Quest 10: C++ Dynamic Batcher

## Interview Prompt

Implement a C++ dynamic batcher for inference requests. Each caller submits one item
and receives a future. The worker groups requests until `max_batch_size` is reached
or `max_wait` expires, then calls `infer_batch` once and maps outputs back to callers.

## Input and Output Contract

The public interface is the declaration shown below. Inputs, return values,
blocking behavior, ownership rules, and shutdown behavior are part of the
contract and are exercised by `test_solution.py` plus `test_driver.cpp`.

```cpp
template <typename T, typename R>
class DynamicBatcher {
 public:
  using BatchFn = std::function<std::vector<R>(const std::vector<T>&)>;

  DynamicBatcher(BatchFn infer_batch,
                 std::size_t max_batch_size,
                 std::chrono::milliseconds max_wait);

  std::future<R> infer(T item);
  void shutdown();
};
```

## Requirements

Implement:

```cpp
template <typename T, typename R>
class DynamicBatcher {
 public:
  using BatchFn = std::function<std::vector<R>(const std::vector<T>&)>;

  DynamicBatcher(BatchFn infer_batch,
                 std::size_t max_batch_size,
                 std::chrono::milliseconds max_wait);

  std::future<R> infer(T item);
  void shutdown();
};
```

Behavior:

- each request gets its own `std::promise` / `std::future`
- the worker batches up to `max_batch_size`
- the worker waits up to `max_wait` for more items
- output `i` resolves request `i`
- exceptions from `infer_batch` propagate to every request in that batch
- shutdown drains accepted work and rejects new work

## System Context

This is the C++ serving version of Quest 02:

```text
client requests
-> queue of request + promise
-> batching worker
-> model infer_batch
-> futures resolved per caller
```

## Why It Exists

Batching can increase GPU/accelerator utilization, but waiting too long hurts tail
latency. This quest forces that latency-throughput tradeoff into code.

## Problem Solved

It turns many small request calls into fewer larger model calls while preserving
per-request results.

## Failure Modes

- resolving the wrong promise with the wrong output
- waiting forever for a full batch
- losing exceptions
- accepting work after shutdown
- increasing p99 latency with an overly large wait

## Metrics

- batch size distribution
- queue wait time
- p50/p95/p99 latency
- throughput
- rejected requests after shutdown

## Improvements After The Basic Version

- cancellation
- timeout futures
- priority requests
- adaptive max wait
- batch-size histograms

## Interview Follow-Ups

- Why does each request need a promise?
- What are the two batch stopping conditions?
- How do you preserve output order?
- What is the latency/throughput tradeoff?

## Player Task

```bash
cd "/Users/yanizhang/Documents/Inference engineer/robotics-inference-lab"
CPP_QUEST_IMPL=starter .venv/bin/python -m pytest -q quests/10-cpp-dynamic-batcher/tests/test_dynamic_batcher.py
```

## Complexity Targets

Let `b` be the number of requests in a batch and `q` the number of accepted,
unresolved requests. `infer` enqueues in amortized `O(1)` time, batch collection and
result or exception delivery cost `O(b)`, and the batcher uses `O(q)` queued space
plus `O(b)` transient batch/input space. These bounds exclude the user-supplied
`infer_batch` runtime and output storage, time blocked on the mutex or condition
variable, thread scheduling, and allocator latency for promises, futures, deque
storage, and transient vectors.

## Interview Follow-ups

1. Which invariant makes this implementation correct?
2. What fails first under overload or malformed input?
3. Which metric would reveal that failure in production?
4. What changes for a robot control loop versus an offline batch job?
