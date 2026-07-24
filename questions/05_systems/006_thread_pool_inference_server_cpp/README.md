# Quest 09: C++ Thread Pool Inference Server

## Interview Prompt

Implement a small thread pool that accepts inference tasks, runs them on worker
threads, returns futures to callers, and shuts down cleanly.

## Requirements

Implement:

```cpp
class ThreadPool {
 public:
  explicit ThreadPool(std::size_t num_workers);

  template <typename Fn>
  auto submit(Fn fn) -> std::future<std::invoke_result_t<Fn>>;

  void shutdown();
};
```

Behavior:

- worker threads start in the constructor
- `submit` returns a future for the task result
- multiple tasks can run concurrently
- shutdown finishes already accepted tasks
- submit after shutdown throws
- destructor shuts down cleanly

## System Context

This sits behind a serving API or preprocessing stage:

```text
request handler
-> thread pool task queue
-> CPU preprocessing / model wrapper / postprocessing
-> response future
```

## Why It Exists

Serving systems often need a bounded number of workers so request concurrency does
not create unbounded OS threads. The task queue gives callers a future while workers
consume jobs.

## Problem Solved

This quest connects request lifecycle, futures, worker threads, and graceful shutdown.

## Failure Modes

- creating one thread per request overloads the process
- missing shutdown can leave dangling threads
- exceptions inside tasks can disappear
- accepting work after shutdown can hang callers

## Metrics

- queued tasks
- active workers
- task latency
- queue wait time
- rejected tasks after shutdown

## Improvements After The Basic Version

- bounded task queue
- task cancellation
- priority tasks
- per-stage timing
- CPU affinity or worker pinning

## Interview Follow-Ups

- Why return a future?
- What happens to queued tasks during shutdown?
- Why not create a new thread for every request?
- Where does this sit relative to dynamic batching?

## Player Task

```bash
cd "/Users/yanizhang/Documents/Inference engineer/robotics-inference-lab"
CPP_QUEST_IMPL=starter .venv/bin/python -m pytest -q quests/09-cpp-thread-pool-inference-server/tests/test_thread_pool.py
```
