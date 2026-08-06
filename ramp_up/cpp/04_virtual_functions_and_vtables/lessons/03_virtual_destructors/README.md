# 03 — Virtual Destructors

## The problem this lesson solves

An owner typed as `std::unique_ptr<Sensor>` may actually hold a `Camera` with
resources that only `Camera` knows how to release.

## The lesson

Polymorphic deletion starts with a base pointer, so the base destructor must be
virtual. Destruction then runs from the most-derived class toward the base:
`Camera` cleans up, its members release their resources, and `Sensor` finishes.

Deleting a derived object through a base pointer whose destructor is not virtual
has undefined behavior; derived cleanup may be skipped, but no particular output
is guaranteed. This lesson teaches that failure in prose only. Its program uses a
safe virtual destructor and `std::unique_ptr<Sensor>` throughout.

## How interviewers test this

**Prediction:** predict the destructor trace through a base owner.

**Implementation:** implement safe observable base destruction.

**Follow-up:** explain the failure mode of a non-virtual base destructor.

**Evidence:** show derived-then-base cleanup in the log.

Expect to spot a polymorphic base without a virtual destructor and explain why a
smart pointer cannot repair an unsafe deletion contract by itself.

## Muscle memory

```cpp
struct Sensor {
    virtual ~Sensor() = default;
};
```

If a type may be destroyed through its interface, make its destructor virtual.

## The drills

Complete the two logging destructors and payload release counter. The evidence
must show derived-then-base cleanup and one released payload.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables/lessons/03_virtual_destructors -q
```

Continue to the [next lesson](../04_polymorphic_ownership/).
