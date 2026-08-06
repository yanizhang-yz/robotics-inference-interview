# 03 — Move Construction

## The problem this lesson solves

A capture-to-inference handoff should not copy every pixel merely because the
destination is a new object.

## The lesson

A `std::vector` is an owning handle for a separate allocation. A move
constructor can initialize the destination by moving that handle, preserving
the allocation address instead of copying bytes. This class defines its source
invariant as a `0x0` buffer after the handoff.

The move constructor is marked `noexcept`; later lessons show why containers
care about that promise.

## How interviewers test this

**Prediction:** predict destination address and source state after construction.

**Implementation:** implement the move constructor.

**Follow-up:** explain `noexcept` and the class's moved-from invariant.

**Evidence:** show address preservation without a pixel copy.

Expect to implement a move constructor, trace the source and destination, and
use `data()` identity to distinguish a handle transfer from a deep copy.

## Muscle memory

```cpp
PixelBuffer input(std::move(captured));
// input owns captured's old allocation; captured is valid and empty
```

## The drills

Implement `PixelBuffer(PixelBuffer&&) noexcept`. Transfer dimensions and the
vector, then establish the documented empty source state.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/lessons/03_move_construction -q
```

Continue to the [next lesson](../04_moved_from_state/).
