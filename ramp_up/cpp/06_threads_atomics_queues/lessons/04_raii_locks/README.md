# 06.4 — RAII lock scopes

After this lesson you can make mutex release unconditional across normal returns,
early returns, and exceptions.

## The problem this lesson solves

Manual `lock()` followed later by `unlock()` leaves a dangerous gap: any exception or
early return in between can keep the mutex locked forever. Other workers then block
with no path to progress.

## The lesson

`std::lock_guard` acquires in its constructor and releases in its destructor. Its
enclosing scope is the critical section.

```cpp
try {
    std::lock_guard<std::timed_mutex> lock(mutex);
    throw std::runtime_error("read failed");
} catch (const std::runtime_error&) {
}
```

Stack unwinding destroys the guard before control enters the handler because the
handler is outside the guard's scope. `try_lock()` can then prove the mutex was
released; a successful probe must itself be followed by `unlock()`.

## How interviewers test this

Expect a function containing manual lock/unlock plus an early return or throw. Explain
which scope owns the lock and the exact point where its destructor runs. A catch block
inside the guarded scope would not yet prove release.

## Muscle memory

Acquire mutexes with an RAII guard immediately. Make the braces match the invariant's
critical section, and never insert manual release paths that must stay in sync.

## The drills

Implement `lock_released_after_exception(std::timed_mutex&)`: acquire through
`lock_guard`, throw, catch outside the guard scope, probe with `try_lock`, and release
the successful probe.

## How to practice

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/04_raii_locks -q
uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/04_raii_locks -q
```
