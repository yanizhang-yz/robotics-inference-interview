# 02 — Temporaries and Value Categories

## The problem this lesson solves

Copy and move overloads use the same call syntax. You need to predict which one
is eligible for a named object, a temporary, and a named object cast with
`std::move`.

## The lesson

An lvalue expression identifies a persistent object. A temporary is an rvalue.
For the non-const expressions in this drill, the overloads `category(const
Frame&)` and `category(Frame&&)` make that routing visible. A name is an lvalue
even when its type is an rvalue reference. The first overload is described by
the reference type it selects, not as a universal lvalue classifier: it can also
bind a const xvalue when no `const Frame&&` overload exists.

`std::move(frame)` is only a cast to an rvalue expression. It does not transfer
or mutate anything by itself; an operation selected afterward may move from it.

## How interviewers test this

**Prediction:** select the reference overload for each non-const expression.

**Implementation:** implement both overload result labels.

**Follow-up:** explain const xvalues and why `std::move` is only a cast.

**Evidence:** apply reference-binding rules without mutating the frame.

You may be asked to classify expressions, select the winning overload, or
correct the claim that `std::move` performs a move.

## Muscle memory

```cpp
category(frame);             // const-reference overload
category(Frame{{1, 2}});     // rvalue-reference overload
category(std::move(frame));  // rvalue-reference overload; cast only
```

## The drills

Implement the two overloads so they report `const-reference overload` and
`rvalue-reference overload`. Keep their reference types unchanged and observe
the address and size checks after the cast-only call.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/lessons/02_temporaries_and_value_categories -q
```

Continue to the [next lesson](../03_move_construction/).
