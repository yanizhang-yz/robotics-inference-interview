# 04 — Moved-From State

## The problem this lesson solves

Move assignment replaces an existing resource and leaves the source object
alive. Both objects still need valid invariants, including during self-move.

## The lesson

The general standard-library guarantee is valid but unspecified: destruction
and assignment are safe, but the old value cannot be assumed. A class can offer
a stronger contract. `PixelBuffer` defines a moved-from buffer as `0x0` and
empty, which makes reassignment straightforward.

A self-move guard prevents an object from clearing the resource it is trying to
transfer to itself.

## How interviewers test this

**Prediction:** trace distinct-source, self-move, and source-reuse cases.

**Implementation:** implement move assignment.

**Follow-up:** separate class guarantees from unspecified library state.

**Evidence:** state and preserve the invariant after every path.

You may be asked what operations are safe on a moved-from object, whether it is
destroyed, and how a move-assignment operator handles self-move.

## Muscle memory

```cpp
target = std::move(source);
source = PixelBuffer(2, 2, 0);  // assigning a new value is safe
```

## The drills

Implement `PixelBuffer& operator=(PixelBuffer&&) noexcept`. Replace the target,
empty the distinct source, preserve validity on self-move, and return `*this`.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/lessons/04_moved_from_state -q
```

Continue to the [next lesson](../05_copy_elision/).
