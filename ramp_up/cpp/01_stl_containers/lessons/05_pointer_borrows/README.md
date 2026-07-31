# 05 — Pointer Borrows

## Problem

Find a named sensor in a robotics registry without copying it, while still
representing that a requested sensor might not exist.

## Mental model

A raw pointer in this narrow modern-C++ role is **nullable, non-owning access**
to an object someone else owns. `find_sensor` does not create a `Sensor`,
destroy one, or transfer ownership. It either borrows access to a matching
`Sensor` already stored in the vector or returns `nullptr` to say “not found.”

`const Sensor*` makes both parts visible:

- `const Sensor` means callers may inspect the borrowed sensor but not modify it
  through this pointer.
- `*` means the result is a pointer, so it may be `nullptr` and must be checked
  before it is used.

When a match is found, `&sensor` takes the address of the `Sensor` currently
being visited. The caller uses `camera->rate_hz` to access a member through the
pointer; `camera->rate_hz` is shorthand for `(*camera).rate_hz`.

This is not an ownership mechanism. Prefer values, references, and smart
pointers for their respective roles; use a raw pointer here because “may be
absent, and is owned elsewhere” is exactly the information the API needs to
communicate.

## Application

An inference service for a robot may keep its camera and joint-encoder
configuration in one sensor registry. A lookup for `"wrist-camera"` returns a
borrowed view of its update rate. A lookup for an optional sensor returns
`nullptr`, allowing the caller to choose a fallback rather than assuming the
hardware is present.

## Validity boundary

The returned pointer remains valid only while the vector exists **and is not
reallocated**. If the vector is destroyed, the pointer dangles. If a later
operation such as `push_back` grows the vector and reallocates its storage, the
elements move and pointers to its old elements dangle too. Do not retain this
borrow across such a change; look it up again after the registry is updated.

## Prediction

Before running the program, answer these questions:

1. What should `find_sensor(sensors, "missing")` return: a `Sensor`, a
   `nullptr`, or an empty string?
2. Why must `camera != nullptr` be checked before evaluating
   `camera->rate_hz`? What does `->` mean here?
3. When the loop finds a match, what object does `&sensor` point to: a new
   copy, the `Sensor` in the vector, or the vector itself?
4. If you save `camera`, then a non-const registry grows and its vector
   reallocates, may you still dereference `camera`? Why or why not?

## Guided implementation

In `starter.cpp`, scan `sensors` with `const Sensor& sensor`. If the current
sensor has the requested name, return `&sensor`. If the loop ends without a
match, return `nullptr`.

## Verification

Run the reference with:

```bash
.venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/05_pointer_borrows -q
```

Run the learner starter with:

```bash
PRACTICE=1 .venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/05_pointer_borrows -q
```

The shared runner compiles with C++20 and `-Wall -Wextra -Werror=return-type`. The
untouched starter is expected to fail at `camera != nullptr`.

## Explain it

- `nullptr` is the explicit “no borrowed object” result.
- `&sensor` borrows the address of the matching vector element.
- `->` accesses a member through a non-null pointer.
- The pointer owns nothing, and vector destruction or reallocation ends this
  borrow’s validity.

## Next connection

Use a nullable borrowed pointer for a lookup that may miss. When an operation
requires a sensor to exist, check the pointer at that boundary, then work with
the proven non-null result.
