# 04 — Polymorphic Ownership

## The problem this lesson solves

A robot needs to own several sensor implementations together without manual
`new`/`delete` or separate containers for every concrete type.

## The lesson

`std::vector` remains homogeneous: every element has exactly the type
`std::unique_ptr<Sensor>`. The objects behind those owners may have different
dynamic types, so one element can own a `Camera` and another a `Lidar`.

Moving a concrete `unique_ptr` into the vector transfers ownership. Borrowing
each owner with `sensor->rate_hz()` keeps ownership in the container while
dispatching through the common interface. When the vector dies, its owners
delete their concrete objects safely through `Sensor`'s virtual destructor.

## How interviewers test this

**Prediction:** predict ownership and dispatch in a mixed base-owner vector.

**Implementation:** implement construction and polling of owned backends.

**Follow-up:** explain move-only storage and cleanup.

**Evidence:** identify one owner per object and the dynamic result for each.

Expect to build a heterogeneous collection with one ownership type and to
explain why raw owning pointers or a vector of base values are inferior choices.

## Muscle memory

```cpp
std::vector<std::unique_ptr<Sensor>> sensors;
sensors.push_back(std::make_unique<Camera>());
```

Store homogeneous smart pointers; let their pointees supply the runtime variety.

## The drills

Implement `make_sensors` and `total_rate`. Preserve exclusive ownership, return
both dynamic types, total 40 Hz, and let scope exit return the live count to zero.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables/lessons/04_polymorphic_ownership -q
```

Continue to the [next lesson](../05_object_slicing/).
