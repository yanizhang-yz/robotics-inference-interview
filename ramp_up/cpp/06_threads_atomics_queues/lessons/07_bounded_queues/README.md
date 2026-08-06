# 06.7 — Bounded blocking queues and backpressure

After this lesson you can derive the two predicates and two notifications of a
bounded camera-to-inference FIFO.

## The problem this lesson solves

If a camera produces faster than inference consumes, an unbounded queue turns the
rate mismatch into growing memory use and stale-frame latency. A bounded queue makes
the mismatch explicit: producers sleep while the queue is full.

## The lesson

`BoundedQueue<T>` protects a deque and its capacity invariant with one mutex. Producers
wait on `not_full_` until `items_.size() < capacity_`; consumers wait on `not_empty_`
until the deque is non-empty. A successful push can enable a pop, and a successful pop
can enable a push, so each operation notifies the opposite condition variable.

Capacity zero is rejected with `std::invalid_argument("capacity must be positive")`.
Without that validation, the not-full predicate could never become true. `size()` also
locks because an observation of shared container state is still a shared access.

The backpressure test fills a capacity-two queue before starting a third push. A
test-only condition-variable handshake reports either full-queue wait registration or
an early producer return. After registration, the test proves the push is incomplete,
pops one item, and joins the producer to prove completion. No sleep, yield deadline,
or assumed schedule decides the result.

## How interviewers test this

**Prediction:** derive full and empty predicates and their enabled transitions.

**Implementation:** implement bounded FIFO push, pop, and size.

**Follow-up:** explain zero capacity and possible shutdown semantics.

**Evidence:** show FIFO, capacity, exact delivery, and registered backpressure.

Derive both predicates, say why predicate waits tolerate spurious wakes, match each
state transition to the waiter it enables, and explain why a single condition variable
would be less expressive. Be ready to defend the zero-capacity policy.

## Muscle memory

For a blocking container, write its invariants and wait predicates before methods.
Mutate while holding the mutex, notify the condition whose predicate may have become
true, and expose no unlocked container queries.

## The drills

Implement the constructor, `push`, `pop`, and `size`. Assertions cover zero capacity,
FIFO, exact once-only delivery, capacity enforcement, and producer backpressure. The
neutral starter never waits, so an incomplete attempt fails quickly.

## How to practice

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/07_bounded_queues -q
uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/07_bounded_queues -q
```

Continue to the [next lesson](../08_clean_shutdown/).
