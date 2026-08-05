# 06.5 — Atomics and compare-and-swap

After this lesson you can use compare-and-swap for one independent state transition
and explain when a mutex is the clearer tool.

## The problem this lesson solves

Several inference workers may finish together, but only one should publish a shared
result. A check followed by a separate write is not atomic: multiple workers could
all observe an unclaimed flag before any stores `true`.

## The lesson

`compare_exchange_strong(expected, desired)` compares and conditionally writes as one
atomic operation. Each worker begins with its own `expected = false`:

```cpp
bool expected = false;
if (claimed.compare_exchange_strong(expected, true)) {
    ++winners;
}
```

One worker changes `claimed` from false to true. Every later comparison fails. On
failure, CAS writes the actual current value back into that worker's `expected`, a
detail interviewers often ask about. Atomics fit this one-field transition; a mutex
is usually clearer when correctness spans several fields or steps.

## How interviewers test this

Explain why the check and claim must be indivisible, why each attempt needs its own
`expected`, and what value `expected` holds after failure. Do not generalize one atomic
flag into protection for unrelated non-atomic state.

## Muscle memory

Use an atomic when the invariant is the atomic object's own independent state. Reach
for a mutex when you need to read or update multiple values consistently.

## The drills

Implement `elect_publisher(threads)` with an atomic claim flag, atomic winner count,
one CAS attempt per worker, and complete joins. Both one and eight workers must produce
exactly one winner.

## How to practice

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/05_atomics -q
uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/05_atomics -q
```
