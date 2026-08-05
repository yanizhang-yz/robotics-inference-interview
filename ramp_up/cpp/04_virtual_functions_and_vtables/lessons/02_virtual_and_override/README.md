# 02 — `virtual` and `override`

## The problem this lesson solves

A sensor backend can compile yet quietly call the wrong function if the base
contract is not virtual or a derived signature misses the contract.

## The lesson

`virtual` on a base member opts that operation into runtime dispatch. `override`
on a derived member asks the compiler to prove that the name, parameters,
qualifiers, and return type match a base virtual operation.

A missing `const`, for example, creates a different member instead of the
intended override. Keeping `override` on every concrete implementation turns
that silent dispatch bug into a compile error at the declaration.

## How interviewers test this

Expect to repair a backend that behaves correctly when called directly but not
through `const Sensor&`, and to explain the different jobs of `virtual` and
`override`.

## Muscle memory

```cpp
std::string name() const override;
```

Put `virtual` on the base contract and `override` on every implementation.

## The drills

Implement `Camera` and `Lidar` while preserving each `override`. The assertions
exercise the concrete behavior only through base references.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables/lessons/02_virtual_and_override -q
```
