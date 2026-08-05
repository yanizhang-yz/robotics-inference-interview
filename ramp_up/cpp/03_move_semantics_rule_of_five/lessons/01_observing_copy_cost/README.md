# 01 — Observing Copy Cost

## The problem this lesson solves

An owning camera frame can contain millions of bytes. A by-value parameter may
silently duplicate that payload when the caller passes a named frame.

## The lesson

`TrackedFrame` counts copy construction. `inspect_by_value(TrackedFrame)`
creates an independent parameter, while
`inspect_by_const_ref(const TrackedFrame&)` borrows the original. Both can read
the same checksum, but only the by-value boundary copies the vector.

Reset counters immediately before the call being measured. Otherwise unrelated
setup can make the evidence ambiguous.

## How interviewers test this

Expect to choose between value and `const&` for a large read-only frame and to
quantify exactly where the copy occurs.

## Muscle memory

```cpp
TrackedFrame::reset_counters();
inspect_by_const_ref(frame);  // read-only borrow, zero copies
```

Use value when the callee needs its own object; use `const&` for a non-owning,
read-only view whose lifetime is limited to the call.

## The drills

Implement both inspection functions. They return the frame checksum. Preserve
the signatures so the assertions expose one copy for value and zero for
`const&`.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/lessons/01_observing_copy_cost -q
```
