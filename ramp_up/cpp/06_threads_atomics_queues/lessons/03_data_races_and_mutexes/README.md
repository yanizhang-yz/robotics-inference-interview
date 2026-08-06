# 06.3 — Data races and mutex boundaries

After this lesson you can define a C++ data race and protect a plain shared invariant
with one explicit mutex boundary.

## The problem this lesson solves

`++counter` is a read-modify-write operation, not an indivisible action. Concurrent
unsynchronized accesses to the same memory, with at least one write, form a data race
and therefore undefined behavior. A lucky correct-looking run proves nothing.

## The lesson

`safe_count_mutex` gives one mutex responsibility for the invariant: every change to
the shared counter happens while that mutex is held.

```cpp
for (int j = 0; j < iterations; ++j) {
    std::lock_guard<std::mutex> lock(mutex);
    ++counter;
}
```

The counter is read only after every worker is joined. The lock orders conflicting
writes; the joins order worker completion before the final read. The test asserts the
invariant's exact result rather than trying to make a racy implementation visibly
fail on one schedule.

## How interviewers test this

**Prediction:** identify the conflicting accesses that form a data race.

**Implementation:** implement the mutex-protected counter.

**Follow-up:** choose lock granularity for a larger invariant.

**Evidence:** state the invariant and show every access is protected.

Define the race precisely, then name the shared invariant and every access it covers.
Be ready to explain why adding sleeps, observing lost updates, or testing many times
cannot make undefined behavior acceptable.

## Muscle memory

List shared writable state first. Assign one synchronization strategy to each
invariant, keep the critical section clear, and join threads before reading their
final results.

## The drills

Implement `safe_count_mutex(threads, iterations)` with a plain `int`, one mutex, a
worker vector, `std::lock_guard`, and complete joins. The assertions require exact
results for four workers and one worker.

## How to practice

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/03_data_races_and_mutexes -q
uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/03_data_races_and_mutexes -q
```

Continue to the [next lesson](../04_raii_locks/).
