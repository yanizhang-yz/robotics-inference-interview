# 02 — Temporaries and Value Categories

## The problem this lesson solves

Copy and move overloads use the same call syntax. You need to predict which one
is eligible for a named object, a temporary, and a named object cast with
`std::move`.

## The lesson

An lvalue expression identifies a persistent object. A temporary is an rvalue.
The overloads `category(const Frame&)` and `category(Frame&&)` make that routing
visible. A name is an lvalue even when its type is an rvalue reference.

`std::move(frame)` is only a cast to an rvalue expression. It does not transfer
or mutate anything by itself; an operation selected afterward may move from it.

## How interviewers test this

You may be asked to classify expressions, select the winning overload, or
correct the claim that `std::move` performs a move.

## Muscle memory

```cpp
category(frame);             // lvalue overload
category(Frame{{1, 2}});     // rvalue overload
category(std::move(frame));  // rvalue overload; cast alone does not mutate
```

## The drills

Implement the two overloads so they report `lvalue` and `rvalue`. Keep their
reference types unchanged and observe the address and size checks after the
cast-only call.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/lessons/02_temporaries_and_value_categories -q
```
