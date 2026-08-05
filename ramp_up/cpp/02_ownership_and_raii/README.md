# Module 02 — Ownership and RAII

This module builds a precise model of object lifetime and cleanup. Work through
the short lessons in order, then combine their ideas in the existing ownership
and RAII capstone.

## Prerequisites

Complete [Module 01](../01_stl_containers/) or be comfortable with values,
references, pointer borrows, classes, and `std::vector`.

## Lesson path

| Lesson | Concept | Application |
| --- | --- | --- |
| [01 — Scope and lifetime](lessons/01_scope_and_lifetime/) | Locals die at scope exit in reverse construction order | Trace nested sensor-session scopes. |
| [02 — Construction and destruction](lessons/02_construction_and_destruction/) | Members construct in declaration order and destruct in reverse | Reason about camera pipeline dependencies. |
| [03 — Owners and borrowers](lessons/03_owners_and_borrowers/) | Containers own; references and pointers borrow | Find and inspect a frame without taking ownership. |
| [04 — `unique_ptr`](lessons/04_unique_ptr/) | Exclusive ownership with destructor cleanup | Build an owned camera frame in a factory. |
| [05 — Ownership transfer](lessons/05_ownership_transfer/) | Moving transfers one cleanup responsibility | Hand a frame from capture to inference. |
| [06 — Span views](lessons/06_span_views/) | Length-carrying borrows of contiguous storage | Mutate and inspect tensor views. |
| [07 — RAII resources](lessons/07_raii_resources/) | Destructors release resources on every exit path | Keep a device open only during a read. |

## Already know ownership and RAII?

Take the fast path if you can answer all five without guessing:

1. Given nested local objects, can you predict their exact destruction order?
2. For a vector element accessed through `Frame*`, which value is the owner and
   which is the borrower—and what can invalidate the borrow?
3. After `auto b = relay(std::move(a));`, which `unique_ptr` owns the object and
   what happened to the object's address?
4. What lifetime and address-stability requirements must a `std::span`'s backing
   storage satisfy?
5. Why does a destructor still clean up during an exception, while an explicit
   cleanup call after the throwing operation may never run?

If any answer is fuzzy, use the corresponding micro-lesson. Otherwise, jump to
the capstone.

## Capstone — Owned buffers and scoped cleanup

The module-root `starter.cpp` combines the concepts in four existing pieces:
`Buffer` owns a heap array, `makeBuffer` returns exclusive ownership,
`moveBuffer` consumes it, and `ScopedLogger` records deterministic scope entry
and exit. Its assertions cover allocation, ownership transfer, and nested RAII
cleanup.

## How to practice

Write attempts in each lesson's `starter.cpp`, then run its `PRACTICE=1`
command. Run every reference lesson plus the capstone with:

```bash
uv run pytest ramp_up/cpp/02_ownership_and_raii -q
```

Practice the unchanged module capstone with:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/02_ownership_and_raii -q
```

Return to the [C++ curriculum](../README.md) or continue to
[Module 03 — Move semantics and the Rule of Five](../03_move_semantics_rule_of_five/).
