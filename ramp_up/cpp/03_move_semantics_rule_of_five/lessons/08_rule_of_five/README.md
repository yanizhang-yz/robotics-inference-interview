# 08 — Rule of Five

## The problem this lesson solves

A low-level wrapper around a raw allocation cannot rely on generated copying:
copying only the pointer would create two owners and a double deletion.

## The lesson

`RawFrame` owns a `new[]` allocation, so its invariant is one object, one buffer.
The destructor releases it; copies allocate independent buffers; moves exchange
the pointer into the destination and empty the source. Copy-and-swap gives copy
assignment cleanup and exception safety. Both assignments handle self-assignment.

This lesson deliberately uses manual allocation to expose the five operations.
Prefer Rule of Zero and standard owning members everywhere else.

## How interviewers test this

Expect to derive all five operations from the ownership invariant, discuss
self-assignment and exception safety, and contrast deep copy with move transfer.

## Muscle memory

```cpp
RawFrame copy(original);             // different address, same bytes
RawFrame moved(std::move(original)); // same address, source empty
```

## The drills

Implement the destructor, copy constructor, copy assignment, move constructor,
and move assignment. Keep `live_allocations` exact and use `swap` for the
copy-and-swap assignment.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/lessons/08_rule_of_five -q
```
