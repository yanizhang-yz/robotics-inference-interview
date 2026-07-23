# Thread-Safe Ring Buffer

**Difficulty:** Medium  
**Source:** Classic systems interview question — asked at robotics and embedded companies in both Python and C++ variants  
**Tags:** `ring-buffer`, `concurrency`, `locks`, `fixed-memory`

## Problem

Implement a **ring buffer** (circular queue) with a fixed capacity, safe to use from
multiple threads:

- `RingBuffer(capacity)` — fixed capacity, no growing, no shrinking.
- `push(item) -> bool` — append to the tail; return `False` (do nothing) if full.
- `push_overwrite(item)` — append to the tail; if full, **evict the oldest** item.
- `pop()` — remove and return the oldest item, or `None` if empty.
- `__len__`, `is_empty()`, `is_full()`.

All operations must be O(1) and thread-safe.

```
rb = RingBuffer(3)
rb.push(1); rb.push(2); rb.push(3)   # -> True, True, True
rb.push(4)                           # -> False (full)
rb.push_overwrite(4)                 # evicts 1
rb.pop()                             # -> 2 (oldest surviving item)
```

## Why this appears in robotics inference interviews

This is *the* data structure between a sensor and a model. A camera produces frames at
30 fps; inference consumes them at 20 fps. An unbounded queue eats memory until the
process dies — a ring buffer caps memory **and** `push_overwrite` implements the policy
real robots want: *drop the oldest frame, keep the freshest*. Fixed capacity also means
no allocation after startup, which matters on embedded targets. Interviewers probe:
index wraparound arithmetic, full-vs-empty disambiguation, and whether you know why the
lock must cover the whole read-modify-write.

## Approach

Fixed list of `capacity` slots + a `head` read index + a `size` count (the count is the
easiest way to distinguish full from empty — both have `head == tail` otherwise).

- `push`: write at `(head + size) % capacity`, increment `size`.
- `pop`: read at `head`, advance `head = (head + 1) % capacity`, decrement `size`.
- `push_overwrite` when full: write at `head`'s slot, advance `head` — one operation,
  eviction and insertion together.
- One `threading.Lock` acquired by every public method (`with self._lock:`). Compute
  fullness inline from `size` — calling your own locked `is_full()` from inside `push`
  would deadlock on a non-reentrant lock.

**Time:** O(1) per operation. **Space:** O(capacity), allocated once.

## Follow-ups to be ready for

1. Make `pop` *block* until an item arrives (condition variable — see the C++ bounded
   queue in `ramp_up/cpp/06_threads_atomics_queues`, which is this question's big sibling).
2. Single-producer/single-consumer without a lock: why head/tail owned by different
   threads (plus memory barriers) can make SPSC lock-free.
3. Why Python's GIL does **not** make the unlocked version safe (interleaving between
   bytecodes still corrupts multi-step updates).
