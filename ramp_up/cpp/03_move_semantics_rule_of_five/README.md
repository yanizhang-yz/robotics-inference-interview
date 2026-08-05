# Module 03 — Move Semantics and the Rule of Five

This module turns ownership transfer into a cost model you can reason about:
which expressions copy, which can move, what remains after a move, and when no
special-member code is the best special-member code.

## Prerequisites

Complete [Module 02](../02_ownership_and_raii/) or be comfortable with RAII,
exclusive ownership, borrows, and transferring a `std::unique_ptr`.

## Lesson path

| Lesson | Concept | Application |
| --- | --- | --- |
| [01 — Observing copy cost](lessons/01_observing_copy_cost/) | Value parameters copy owning payloads; `const&` borrows | Measure a large frame inspection boundary. |
| [02 — Temporaries and value categories](lessons/02_temporaries_and_value_categories/) | Names are lvalues; temporaries and casts are rvalues | Predict overload selection and demystify `std::move`. |
| [03 — Move construction](lessons/03_move_construction/) | New objects can transfer resource handles | Hand a capture buffer to inference without copying pixels. |
| [04 — Moved-from state](lessons/04_moved_from_state/) | Moved-from objects remain valid | Define an empty invariant, survive self-move, and reuse a source. |
| [05 — Copy elision](lessons/05_copy_elision/) | C++17 prvalues construct directly in place | Return frames by value with zero copy/move operations. |
| [06 — `noexcept` moves](lessons/06_noexcept_moves/) | Containers rely on non-throwing moves | Prevent vector growth from falling back to copies. |
| [07 — Rule of Zero](lessons/07_rule_of_zero/) | Owning standard members compose correct special members | Copy and move a frame batch without custom machinery. |
| [08 — Rule of Five](lessons/08_rule_of_five/) | Raw ownership requires all five operations | Build a deep-copying, movable low-level frame wrapper. |

## Already know move semantics?

Take the fast path only if you can answer these without guessing:

1. Which overload accepts a named `Frame`: `const Frame&` or `Frame&&`? Which
   accepts `Frame{}` and `std::move(frame)`?
2. If a moved destination has the source vector's old `data()` address, what
   work did the move avoid?
3. What does valid but unspecified permit you to do with a moved-from standard
   object, and what does it forbid you from assuming?
4. Why can a C++17 function return `Frame(width, height)` with zero observed
   copies and moves, and why can `return std::move(local)` be worse?
5. Why might `std::vector` copy a type that has a move constructor, and how does
   `noexcept` change that decision?
6. When should a class use the Rule of Zero, and what ownership condition
   forces a low-level class toward the Rule of Five?

If an answer is fuzzy, use the corresponding micro-lesson. Otherwise, continue
to the capstone.

## Capstone — Frame handoffs

The unchanged module-root capstone combines the ideas in an instrumented
`FrameBuffer`. Implement its copy and move operations, `make_frame`, `consume`,
and `swap_frames`. Existing assertions prove deep-copy independence, allocation
address preservation during moves, the defined empty source state, copy
elision, caller-selected consumption cost, and a three-move swap.

## How to practice

Write attempts in each lesson's `starter.cpp`, then run its `PRACTICE=1`
command. Run every reference lesson plus the capstone with:

```bash
uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five -q
```

Practice the module capstone with:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five -q
```

Return to the [C++ curriculum](../README.md) or continue to
[Module 04 — Virtual functions and vtables](../04_virtual_functions_and_vtables/).
