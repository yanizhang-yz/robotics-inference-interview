# Quest 09: C++ Thread Pool Inference Server

## Interview Prompt

Implement a small thread pool that accepts inference tasks, runs them on worker
threads, returns futures to callers, and shuts down cleanly.

## Input and Output Contract

The public interface is the declaration shown below. Inputs, return values,
blocking behavior, ownership rules, and shutdown behavior are part of the
contract and are exercised by `test_solution.py` plus `test_driver.cpp`.

```cpp
class ThreadPool {
 public:
  explicit ThreadPool(std::size_t num_workers);

  template <typename Fn>
  auto submit(Fn fn) -> std::future<std::invoke_result_t<Fn>>;

  void shutdown();
};
```

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

Run the starter implementation:

```bash
PRACTICE=1 uv run pytest questions/05_systems/006_thread_pool_inference_server_cpp -q
```

Run the reference implementation:

```bash
uv run pytest questions/05_systems/006_thread_pool_inference_server_cpp -q
```

## Complexity Targets

For `w` workers and `q` queued tasks, construction performs `O(w)` thread setup,
`submit` performs `O(1)` expected queue work, and shutdown performs `O(w + q)` pool
bookkeeping while waiting for all accepted task runtimes to finish. Retained pool
overhead is `O(w + q)` plus the storage owned by user callables and future results.
These bounds exclude task execution time, mutex contention, condition-variable
wake-up, thread creation/join and OS scheduling latency, and per-submission
allocation latency for the packaged task, shared ownership, callable wrapper, future
state, and queue storage.

## Interview Follow-ups

1. Which invariant makes this implementation correct?
2. What fails first under overload or malformed input?
3. Which metric would reveal that failure in production?
4. What changes for a robot control loop versus an offline batch job?
