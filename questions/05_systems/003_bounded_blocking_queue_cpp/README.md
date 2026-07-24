# Quest 04: C++ Bounded Blocking Queue

## Interview Prompt

Design and implement a thread-safe bounded blocking queue for an inference server.

Multiple client threads push requests into the queue. Worker threads pop requests from
the queue and run inference. The queue must provide backpressure when it is full,
block consumers when it is empty, and shut down cleanly.

## Input and Output Contract

The public interface is the declaration shown below. Inputs, return values,
blocking behavior, ownership rules, and shutdown behavior are part of the
contract and are exercised by `test_solution.py` plus `test_driver.cpp`.

```cpp
template <typename T>
class BoundedBlockingQueue {
 public:
  explicit BoundedBlockingQueue(std::size_t capacity);

  bool push(T item);
  std::optional<T> pop();
  void close();

  std::size_t size() const;
  std::size_t capacity() const;
  bool closed() const;
};
```

## Requirements

Implement:

```cpp
template <typename T>
class BoundedBlockingQueue {
 public:
  explicit BoundedBlockingQueue(std::size_t capacity);

  bool push(T item);
  std::optional<T> pop();
  void close();

  std::size_t size() const;
  std::size_t capacity() const;
  bool closed() const;
};
```

Behavior:

- `push(T item)` blocks while the queue is full.
- `push(T item)` returns `false` if the queue is closed before the item is accepted.
- `pop()` blocks while the queue is empty.
- `pop()` returns `std::nullopt` only when the queue is closed and empty.
- `close()` wakes blocked producers and consumers.
- Existing queued items can still be drained after `close()`.
- New items are not accepted after `close()`.
- FIFO order is preserved.
- Move-only values such as `std::unique_ptr<T>` are supported.
- Multiple producers and multiple consumers are supported.
- The implementation must not busy wait.

## System Context

This queue sits between request intake and inference workers:

```text
HTTP/gRPC clients
-> request parser
-> bounded blocking queue
-> worker threads or dynamic batcher
-> model runtime
-> response path
```

In a robotics system, the same pattern appears between sensors, policy inference, and
control actions:

```text
camera / robot state stream
-> observation queue
-> policy worker
-> action queue
-> robot control loop
```

## Real Coding Scenarios

### Scenario 1: HTTP Inference Server

In an inference server, request-handler threads are producers. Worker threads are
consumers.

```cpp
struct Request {
  int request_id;
  std::vector<float> input;
};

BoundedBlockingQueue<Request> queue(128);

// Producer: called by an HTTP/gRPC handler thread.
bool accepted = queue.push(Request{
    .request_id = request_id,
    .input = preprocess(body),
});

// Consumer/worker: runs on a fixed worker thread.
while (std::optional<Request> request = queue.pop()) {
  Output output = run_model(request->input);
  send_response(request->request_id, output);
}
```

The queue protects the model runtime from unbounded request bursts. If the model
workers cannot keep up, `push` blocks and creates backpressure.

### Scenario 2: Dynamic Batching Server

In a batching server, request-handler threads push request records into a queue. The
batching worker is the consumer.

```cpp
struct PendingRequest {
  Tensor input;
  std::promise<Tensor> result;
};

BoundedBlockingQueue<PendingRequest> queue(1024);

// Producer: each caller submits one request and keeps the future.
std::promise<Tensor> promise;
std::future<Tensor> future = promise.get_future();
queue.push(PendingRequest{std::move(input), std::move(promise)});

// Consumer/worker: gathers several requests into one batch.
std::vector<PendingRequest> batch;
while (batch.size() < max_batch_size) {
  auto request = queue.pop();
  if (!request.has_value()) {
    break;
  }
  batch.push_back(std::move(*request));
}

std::vector<Tensor> outputs = infer_batch(inputs_from(batch));
for (std::size_t i = 0; i < batch.size(); ++i) {
  batch[i].result.set_value(std::move(outputs[i]));
}
```

Here the queue is not the batcher by itself. It is the handoff point between many
request producers and the single batching worker that decides when to run the model.

### Scenario 3: Robot Policy Pipeline

In a robot system, the camera or state-estimation thread can be a producer. The
policy thread is a consumer.

```cpp
struct Observation {
  Image image;
  JointState joints;
  TimePoint timestamp;
};

BoundedBlockingQueue<Observation> observation_queue(4);

// Producer: camera/state callback thread.
observation_queue.push(Observation{
    .image = latest_image,
    .joints = latest_joints,
    .timestamp = now(),
});

// Consumer/worker: policy inference thread.
while (auto observation = observation_queue.pop()) {
  ActionChunk actions = policy_infer(*observation);
  action_queue.push(std::move(actions));
}
```

The small capacity is intentional. A robot usually does not want to process very old
observations. If the policy worker is too slow, the queue depth and push wait time
make that delay visible.

### Producer, Consumer, Worker

The same thread can be a producer for one queue and a consumer for another:

```text
camera thread        -> producer for observation_queue
policy worker        -> consumer of observation_queue
policy worker        -> producer for action_queue
control loop thread  -> consumer of action_queue
```

`worker` describes the thread's job. `producer` and `consumer` describe how the thread
interacts with a particular queue.

## Why It Exists

The queue gives producers and consumers a safe handoff point. It absorbs small bursts
of requests without losing data, and its fixed capacity creates backpressure when the
system is overloaded.

## Problem Solved

Without a queue, producers and workers must coordinate directly. That makes request
handoff fragile and hard to scale.

Without a bound, the system may keep accepting requests until memory grows without
limit. This hides overload until latency and memory fail badly.

## Design Tradeoffs

Bounded queues improve memory predictability and overload behavior, but producers may
wait when the system is saturated. In a serving system, that waiting is intentional:
it is the signal that downstream inference capacity is full.

## Failure Modes

- Missing locks can cause data races and corrupted queue state.
- Sleep polling wastes CPU and adds unpredictable latency.
- Forgetting to wake blocked threads on shutdown can deadlock the server.
- Accepting new items after shutdown can lose requests or hang callers.
- An unbounded queue can hide overload until memory is exhausted.

## Metrics

Useful production metrics:

- queue depth
- push wait time
- pop wait time
- rejected pushes after close
- number of blocked producers
- number of blocked consumers
- request age inside the queue

## Improvements After The Basic Version

- timeout-based `push_for(...)` and `pop_for(...)`
- cancellation tokens
- priority queues for urgent requests
- per-tenant fairness
- queue-depth histograms
- lock-free single-producer/single-consumer queue for simpler sensor paths

## Interview Follow-Ups

- Why bounded instead of unbounded?
- Why use `std::condition_variable` instead of sleeping in a loop?
- What should happen if `close()` is called while producers are blocked?
- What should happen if `close()` is called while consumers are blocked?
- How does this queue create backpressure?
- Where would this queue sit inside a dynamic batching system?
- What would you measure in production?

## Player Task

Fill in `starter.hpp`.

Run the player implementation:

```bash
cd "/Users/yanizhang/Documents/Inference engineer/robotics-inference-lab"
CPP_QUEST_IMPL=starter .venv/bin/python -m pytest -q quests/04-cpp-bounded-blocking-queue/tests/test_bounded_blocking_queue.py
```

Run the reference implementation:

```bash
cd "/Users/yanizhang/Documents/Inference engineer/robotics-inference-lab"
.venv/bin/python -m pytest -q quests/04-cpp-bounded-blocking-queue/tests/test_bounded_blocking_queue.py
```

## Core Mental Model

```text
Producer thread:
  wait until queue has space or queue is closed
  if closed, fail
  push item
  wake one consumer

Consumer thread:
  wait until queue has item or queue is closed
  if item exists, pop it
  if closed and empty, stop
  wake one producer
```

The important interview phrase:

```text
The condition variable does not store the condition. The predicate does.
```

That means every wait must check a real shared state predicate such as:

```cpp
closed_ || items_.size() < capacity_
closed_ || !items_.empty()
```

## Complexity Targets

Once its wait predicate is satisfied, each `push`, `pop`, and state query performs
`O(1)` queue work; `close` performs `O(1)` shared-state work and wakes all waiters.
The queue retains `O(capacity)` elements. These bounds exclude unbounded blocking
time, mutex contention, condition-variable wake-up and OS scheduling costs, and
allocator latency or element move/destruction costs from the underlying `std::deque`;
the logical capacity bounds elements but does not promise a preallocated deque.

## Interview Follow-ups

1. Which invariant makes this implementation correct?
2. What fails first under overload or malformed input?
3. Which metric would reveal that failure in production?
4. What changes for a robot control loop versus an offline batch job?
