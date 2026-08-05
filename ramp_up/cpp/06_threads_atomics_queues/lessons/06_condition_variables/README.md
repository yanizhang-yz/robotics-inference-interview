# 06.6 — Condition variables and predicates

After this lesson you can name the protected state, predicate, state transition, and
notification that make a condition-variable handoff correct.

## The problem this lesson solves

A consumer should sleep while no camera sample exists, without busy-waiting and
without missing a sample delivered just before it begins to wait. Condition variables
can also wake spuriously, so a bare `wait(lock)` is not a correctness condition.

## The lesson

`SampleMailbox<T>` keeps an `optional<T>` behind one mutex. The producer stores the
value while holding the mutex, releases the lock, then calls `notify_one`. The consumer
uses the state predicate directly:

```cpp
std::unique_lock<std::mutex> lock(mutex_);
ready_.wait(lock, [this] { return value_.has_value(); });
```

The predicate is checked while the mutex is held, before sleeping and after every
wake. Once true, the consumer moves the value out and resets the optional. The locked
`empty()` query lets the exercise verify that reset without introducing an unlocked
read of shared state.

## How interviewers test this

Given a condition-variable loop, identify the mutex, predicate, data change, and
notification. Explain why the predicate handles both spurious wakeups and a notify
that happened before the consumer reached `wait`.

## Muscle memory

Write the predicate first. Protect every read and write of the predicate's state with
the same mutex. Change state before notifying, and use `unique_lock` for waiting.

## The drills

Implement `put`, `wait_and_take`, and `empty`. A producer/consumer exchange must
deliver `42`, and the mailbox must be empty after the take. The incomplete starter's
take returns immediately so mistakes fail an assertion instead of hanging.

## How to practice

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/06_condition_variables -q
uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/06_condition_variables -q
```
