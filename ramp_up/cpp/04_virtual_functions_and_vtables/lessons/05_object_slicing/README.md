# 05 — Object Slicing

## The problem this lesson solves

A by-value parameter typed as a base class can accept a derived object yet lose
the very runtime behavior the caller expected.

## The lesson

Passing `CameraRecord` to `kind_by_value(SensorRecord)` constructs a separate
`SensorRecord`. Only the base subobject fits in that parameter; the camera part is
sliced away. Calling its virtual `kind()` correctly reports `sensor` because the
new parameter really is only a base object.

`kind_by_reference(const SensorRecord&)` creates no base copy. The reference
still denotes the original `CameraRecord`, so dynamic dispatch reports `camera`.

## How interviewers test this

Expect to diagnose a virtual call that still reaches the base implementation.
Check whether a derived object crossed a base-class boundary by value before
blaming the vtable.

## Muscle memory

```cpp
std::string kind_by_reference(const SensorRecord& sensor);
```

Use a base reference or smart pointer to preserve polymorphism; avoid base values
for interchangeable runtime implementations.

## The drills

Implement the derived `kind()` plus the by-value and by-reference functions.
Observe that the signatures alone determine whether the camera part survives.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables/lessons/05_object_slicing -q
```
