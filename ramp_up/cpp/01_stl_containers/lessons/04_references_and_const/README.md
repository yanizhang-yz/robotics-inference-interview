# 04 — References and Const

## Card

A reference is a **non-owning alias** for an existing object — the `&` in
the declaration does not create a second object.

```cpp
double max_abs_position(const JointState& state);  // read-only alias
void   clamp_in_place(JointState& state, double);  // mutable alias
```

`const T&` can inspect the caller's object but not change it. `T&` changes
the caller's object directly. Plain `T` copies, so the caller never sees
your edits.

## Predict

`clamp_in_place` must update the caller's joint positions. Which parameter
form does that require?

- A) `JointState state`
- B) `const JointState& state`
- C) `JointState& state`

<!-- predict
answer: C
why-A: By-value receives a copy — the clamping happens to the copy and the caller's positions never change.
why-B: A const reference sees the caller's object but the compiler rejects any attempt to modify through it.
why-C: Right — a mutable reference aliases the caller's own object, so writes through it land in the original.
-->

## Drill

In `starter.cpp`, implement `max_abs_position` by scanning positions
through the read-only reference and returning the greatest absolute
position, then implement `clamp_in_place` by updating each position
through the mutable reference with `std::clamp`.

Manual check: `PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/04_references_and_const -q`

## Takeaway

- `const T&` reads the caller's object without copying or mutating it.
- `T&` mutates the caller's object without copying it.
- The object is the referent; `&` is declaration syntax saying "this
  parameter is an alias for it."

## Deep dive

The referent is the actual object being aliased — here, the caller's
`JointState`. Reference syntax appears only in the declaration
(`const JointState& state`); no second `JointState` exists and there is
nothing separate to manage or free. That makes references the cheapest way
to hand a large object to a function: no copy, no ownership transfer, no
null case to check.

The `const` is a promise enforced by the compiler. `max_abs_position` can
walk every element of `state.positions` but any assignment through the
reference is a compile error — so a reader of the signature alone knows
the call is safe. `clamp_in_place` drops the `const` because mutation is
the point: robotics and inference code often reads a joint state to choose
a safe action, then adjusts that same state so commanded positions stay
inside safety limits before they reach a controller.

The convention to carry forward: pass `const T&` for inspection, reserve
`T&` for operations whose purpose is an in-place update, and let plain `T`
signal "I need my own copy." Parameter lists then document ownership and
mutation intent at every robotics and inference boundary without a comment
in sight.
