# 01 — Interfaces and Dynamic Dispatch

## The problem this lesson solves

A robot may poll cameras, lidars, and future sensors in one loop. The loop needs
one stable contract instead of a branch for every concrete backend.

## The lesson

`Sensor` is an abstract interface: its pure virtual `name()` and `rate_hz()`
describe what callers may ask without choosing an implementation. A
`const Sensor*` can point to either a `Camera` or a `Lidar`; a virtual call then
uses the object's dynamic type to select the matching override.

The pointer's declared `Sensor*` type is its static type. The concrete object it
currently addresses supplies its dynamic type. `poll` depends only on the
interface, so adding another sensor does not change its loop.

## How interviewers test this

Expect to distinguish an interface from an implementation and to explain why a
base pointer or reference can preserve runtime polymorphism while a plain base
value cannot.

## Muscle memory

```cpp
struct Sensor {
    virtual ~Sensor() = default;
    virtual std::string name() const = 0;
};
```

Use a base reference or pointer when one call site must accept interchangeable
runtime implementations.

## The drills

Implement two sensor backends and `poll`. Preserve the abstract `Sensor`
contract and return readings in the same order as the borrowed pointers.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables/lessons/01_interfaces_and_dynamic_dispatch -q
```
