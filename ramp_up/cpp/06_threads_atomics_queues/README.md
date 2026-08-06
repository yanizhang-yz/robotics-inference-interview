# 06 — Threads, Atomics, and Queues

This module builds the synchronization protocol behind a robotics inference pipeline:
a camera produces samples, inference consumes them, bounded capacity applies
backpressure, and shutdown wakes blocked consumers. It relies on Module 02's RAII
ownership model and Module 03's move transfers.

## Start with the diagnostic

Answer these aloud before opening a lesson. Route any uncertain answer through the
lesson map below.

1. A `std::thread` has finished running but was never joined. Is it still joinable,
   and what happens if its thread object is destroyed?
2. A worker lambda is created inside a loop. Which values should be captured by value,
   which shared objects may be captured by reference, and what proves their lifetime?
3. Precisely define a C++ data race. Why is a correct-looking result from one run not
   evidence that unlocked shared writes are safe?
4. What code region does a `std::lock_guard` protect, and why does it release the mutex
   after an exception or early return?
5. When is an atomic the right abstraction, and when does a multi-field invariant call
   for a mutex instead?
6. For a condition-variable wait, can you name the mutex, predicate, protected state
   transition, and notification? Why must the wait recheck a predicate?
7. How does a bounded queue turn a producer/consumer rate mismatch into backpressure?
   What are its two predicates and two notifications?
8. Which state makes shutdown observable to a sleeping consumer, why must close wake
   every blocked consumer, and when may `pop()` return end-of-stream?

## Lesson map

| Lesson | Concept | Application |
| --- | --- | --- |
| [01 — Thread lifetime](lessons/01_thread_lifetime/) | Joinable threads need an owner and join boundary | Sum sensor ranges with two workers. |
| [02 — Lambda captures](lessons/02_lambda_captures/) | Capture mode controls sharing and lifetime | Write ordered sample offsets into disjoint slots. |
| [03 — Data races and mutexes](lessons/03_data_races_and_mutexes/) | A mutex protects a shared invariant | Count completed inference jobs exactly. |
| [04 — RAII locks](lessons/04_raii_locks/) | Guard lifetime makes release unconditional | Preserve a protected probe across exceptions. |
| [05 — Atomics](lessons/05_atomics/) | Atomics suit one independent transition | Elect exactly one publisher with compare-exchange. |
| [06 — Condition variables](lessons/06_condition_variables/) | Predicate waits block on protected state | Deliver one sample to a waiting consumer. |
| [07 — Bounded queues](lessons/07_bounded_queues/) | Two predicates express delivery and backpressure | Connect camera production to bounded inference. |
| [08 — Clean shutdown](lessons/08_clean_shutdown/) | Closure is a wakeable terminal state | Drain work and release all consumers. |

## The camera-to-inference mental model

The finished protocol has four layers:

1. Thread lifetime keeps borrowed inputs alive until workers join.
2. A mutex protects compound queue state, and RAII makes release unconditional.
3. Condition-variable predicates sleep producers or consumers until a state transition
   can make progress.
4. Closure is a terminal transition: accepted items drain first, then consumers receive
   `nullopt`; `notify_all` ensures no empty-queue consumer sleeps forever.

The bounded lesson deliberately rejects capacity zero. Otherwise
`items.size() < capacity` can never become true. Its producer waits for
`items.size() < capacity` and notifies `not_empty` after pushing; its consumer waits
for `!items.empty()` and notifies `not_full` after popping.

The shutdown lesson is deliberately unbounded. It has no producer-capacity wait and
therefore no `not_full` condition. Its consumer predicate is
`closed || !items.empty()`. After that predicate succeeds, data takes priority over
end-of-stream so the queue drains before returning `nullopt`.

## Testing concurrency without testing the scheduler

Reference assertions cover invariants and protocol outcomes: exact counts, ordered
delivery, once-only delivery, capacity bounds, blocked-producer progress, post-close
rejection, draining, and waiter exit. They do not require a particular worker order or
a data race to visibly lose updates. Test-only condition-variable handshakes observe
wait registration before the triggering put, pop, or close. Worker joins prove
completion without polling the scheduler.

The practice mailbox and queue methods return neutral values immediately until you
implement them. That choice is intentional: an incomplete exercise fails an assertion
instead of hanging inside a placeholder wait.

## Capstone

After the eight micro-lessons, return to [starter.cpp](starter.cpp). The capstone keeps
four established pieces together:

- `racy_increment_demo`, whose undefined result is printed but never asserted;
- `safe_count_mutex`, protecting a plain counter with one mutex;
- `safe_count_atomic`, protecting one independent counter transition; and
- `BoundedQueue`, combining predicates, notifications, FIFO delivery, and backpressure.

The capstone is not a shutdown queue; Lesson 08 supplies that final protocol layer.

## Run the module

Reference solutions:

```bash
uv run pytest ramp_up/cpp/06_threads_atomics_queues/test_solution.py -q
```

One practice lesson:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/07_bounded_queues -q
```

Capstone practice:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/test_solution.py -q
```

Previous: [Module 05 — Memory layout and cache behavior](../05_memory_layout_and_cache/)
· Next: [C++ curriculum overview](../README.md).
