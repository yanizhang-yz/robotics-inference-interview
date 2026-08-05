# 06 — `noexcept` Moves

## The problem this lesson solves

A type can have a move constructor yet still be copied when a vector grows.
That surprise usually comes from a missing non-throwing guarantee.

## The lesson

During reallocation, `std::vector` must preserve its original elements if
construction of the new storage fails. It can safely transfer existing elements
when their move constructor is `noexcept`; otherwise it may copy them to retain
the strong exception guarantee.

A `noexcept(...)` expression queries that property at compile time without
constructing an object.

## How interviewers test this

You may be shown unexpected copies during vector growth and asked to inspect the
move signature, or asked what semantic promise `noexcept` makes to containers.

## Muscle memory

```cpp
static_assert(noexcept(Packet(std::declval<Packet&&>())));
```

## The drills

Implement `Packet(Packet&&) noexcept` by moving its byte vector and counting the
operation. Implement `packet_move_is_noexcept()` with a `noexcept(...)`
expression. Growth must record one move and zero copies.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/lessons/06_noexcept_moves -q
```
