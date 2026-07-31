# 04 — References and Const

## Problem

Inspect a robot joint state without copying it, then safely clamp the same
state before it is sent to a controller.

## Mental model

A reference is a **non-owning alias** for an existing object. The referent is
the actual object being aliased: here, the caller's `JointState state`.
Reference syntax appears in a declaration such as `const JointState& state`;
the `&` does not make a second `JointState`, and it is not a separate object
to manage.

`const T&` is a non-owning, read-only alias. `max_abs_position` can inspect
the caller's joint positions but cannot change them. `T&` is a non-owning,
mutable alias. `clamp_in_place` changes the caller's `JointState` directly.

## Application

Inference and robotics code often reads a joint state to choose a safe action,
then adjusts that same state to keep commanded positions within safety limits.
Use `const JointState&` for inspection and `JointState&` only when the
operation intentionally changes the caller-owned state.

## Prediction

Before running the program, predict whether caller state changes when a
function receives `JointState state` (`T`), `const JointState& state`, or
`JointState& state` (`T&`). Which form can `max_abs_position` use, and which
form must `clamp_in_place` use to update the original positions?

## Guided implementation

In `starter.cpp`, implement `max_abs_position` by scanning positions through
the read-only reference and returning the greatest absolute position. Then
implement `clamp_in_place` by updating each position through the mutable
reference with `std::clamp`.

## Verification

Run the reference with:

```bash
.venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/04_references_and_const -q
```

Run the learner starter with:

```bash
PRACTICE=1 .venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/04_references_and_const -q
```

The shared runner compiles with C++20 and `-Wall -Wextra -pedantic`. The
untouched starter is expected to fail at the maximum-position assertion before
it reaches the clamping assertion.

## Explain it

- `const T&` reads the caller's object without copying or mutating it.
- `T&` mutates the caller's object without copying it.
- The object is the referent; `&` is declaration syntax that says the
  parameter is an alias for that object.

## Next connection

Reference parameters make ownership and mutation explicit at robotics and
inference boundaries: inspect shared input with `const T&`, and reserve `T&`
for deliberate in-place safety updates.
