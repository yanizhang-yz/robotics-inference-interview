# 05 — Copy Elision

## The problem this lesson solves

Returning a frame by value looks like it must copy or move a large payload, and
that misconception can produce awkward output parameters or pessimizing casts.

## The lesson

Since C++17, returning a prvalue such as `Frame(width, height)` constructs the
result directly in its destination. No intermediate object needs a copy or move
constructor. Instrumented counters make that absence observable.

Do not write `return std::move(local)` to help the compiler. For a named local,
that cast can prevent named return value optimization; ordinary `return local`
already permits it.

## How interviewers test this

**Prediction:** predict copy and move counters for the returned prvalue.

**Implementation:** implement `make_frame`.

**Follow-up:** explain when `return std::move(local)` can inhibit elision.

**Evidence:** use zero counter changes and correct dimensions as evidence.

Expect to explain guaranteed prvalue elision, distinguish it from optional
NRVO, and identify why a moved return can be worse.

## Muscle memory

```cpp
Frame make_frame(int width, int height) {
    return Frame(width, height);
}
```

## The drills

Implement `make_frame(int, int)` as a returned `Frame` prvalue. The dimensions
must be correct while both instrumentation counters remain zero.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/lessons/05_copy_elision -q
```

Continue to the [next lesson](../06_noexcept_moves/).
