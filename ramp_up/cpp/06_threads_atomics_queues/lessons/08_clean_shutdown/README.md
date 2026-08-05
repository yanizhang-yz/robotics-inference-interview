# 06.8 — Clean queue shutdown

After this lesson you can treat shutdown as part of a queue's synchronization
protocol so consumers drain existing work and then stop without hanging.

## The problem this lesson solves

A consumer waiting only for `!items.empty()` can sleep forever after producers exit.
Destroying threads or process state does not create a wake-up protocol. The queue must
represent closure, include it in the wait predicate, and notify every possible waiter.

## The lesson

`ClosableQueue<T>` is unbounded: producers never wait for capacity. `push` holds the
mutex, rejects work after closure, appends accepted work, and wakes one consumer.
`pop` waits for either terminal state or data:

```cpp
not_empty_.wait(lock, [this] { return closed_ || !items_.empty(); });
```

After waking, `pop` returns the oldest queued item whenever one exists. Only an empty,
closed queue returns `std::nullopt`, so closure drains buffered work before signaling
end-of-stream. `close` sets `closed_` under the mutex and calls `notify_all`; every
blocked consumer must reevaluate the now-true predicate.

The wake-all test uses a private waiter count exposed only through a friend test probe.
The probe locks the queue mutex, so observing all three registered consumers proves
they have entered the empty-queue wait and released that mutex. Only then does the test
close the queue and join all three consumers. No sleep or assumed schedule stands in
for wait registration.

## How interviewers test this

Expect questions about close-versus-push races, why queued items are checked before
returning `nullopt`, and why shutdown requires `notify_all`. Also explain why this
unbounded queue has no `not_full_` condition or producer shutdown wake-up.

## Muscle memory

Put terminal state in the same mutex-protected state machine as normal work. Include
terminal state in every relevant predicate, define drain semantics explicitly, and
wake the complete waiter set when terminal state changes.

## The drills

Implement `push`, `pop`, and `close`. Assertions cover post-close rejection, FIFO
draining, terminal `nullopt`, and multiple consumers returning after closure. The
neutral starter methods return immediately so practice failures cannot deadlock.

## How to practice

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/08_clean_shutdown -q
uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/08_clean_shutdown -q
```
