# 04 — Virtual Functions and Vtables

Runtime polymorphism lets one robotics pipeline choose among camera, lidar, and
inference backends at runtime while the calling code depends on one interface.
This module turns that broad topic into six small programs before returning to
the existing capstone.

## What this module consumes and produces

You will reuse Module 02's `std::unique_ptr` ownership and Module 03's reasoning
about move-only values. This module produces a safe interface-and-dispatch model
for backend abstractions: how to call, own, destroy, and batch runtime-selected
implementations.

## Lesson path

1. [Interfaces and dynamic dispatch](lessons/01_interfaces_and_dynamic_dispatch/README.md)
   separates a common contract from concrete sensors.
2. [`virtual` and `override`](lessons/02_virtual_and_override/README.md) makes
   runtime selection intentional and signature mistakes compile-time errors.
3. [Virtual destructors](lessons/03_virtual_destructors/README.md) proves safe
   derived-to-base cleanup through an interface owner.
4. [Polymorphic ownership](lessons/04_polymorphic_ownership/README.md) combines
   heterogeneous pointees with a homogeneous vector of `unique_ptr<Sensor>`.
5. [Object slicing](lessons/05_object_slicing/README.md) contrasts a lossy base
   value with a polymorphic base reference.
6. [Dispatch cost](lessons/06_dispatch_cost/README.md) places one virtual call
   around coarse batch work and verifies the boundary with counters.

## Diagnostic model

Use this sequence when polymorphic code returns the wrong answer or cleans up
incorrectly:

1. Identify the static type at the call site and the dynamic type of the object.
   A base reference or pointer can preserve the dynamic type; a copied base value
   cannot.
2. Check that the base operation is `virtual`. Without it, the static type chooses
   the function and a derived member merely hides the base member.
3. Check that every derived implementation says `override`. A missing `const` or
   parameter mismatch should fail compilation instead of silently creating a new
   function.
4. Check the destruction boundary. Anything deleted through a polymorphic base
   needs a virtual base destructor, including deletion performed by
   `std::unique_ptr<Base>`.
5. Check for slicing. Passing or storing a derived object as a base value removes
   the derived part before dispatch occurs.
6. Check dispatch granularity. Virtual selection has a small per-call cost and may
   inhibit inlining, so select a backend once around a frame or batch rather than
   once per inner-loop element. Measure call shape with counters before reaching
   for timing claims.

## Capstone drill

The module-root `starter.cpp` keeps the original `Sensor`, `Camera`, `Lidar`,
`pollAll`, `describe`, destructor-order, and forgotten-`virtual` drills. Complete
its TODOs after the micro-lessons:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables -q
```

The reference suite, including all six lessons and the capstone, runs with:

```bash
uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables -q
```
