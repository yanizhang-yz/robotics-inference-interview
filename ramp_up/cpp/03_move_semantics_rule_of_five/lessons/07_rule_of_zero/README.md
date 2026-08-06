# 07 — Rule of Zero

## The problem this lesson solves

Hand-written special members duplicate resource-management logic and easily
introduce leaks, double frees, or inconsistent copy and move behavior.

## The lesson

`std::string` and `std::vector` already implement destruction, copying, and
moving. A class made only from such members should usually declare none of the
five special members: the compiler-generated operations compose their correct
behavior. That is the Rule of Zero.

Passing `FrameBatch` by value gives `relabel_copy` an independent batch to edit.
Moving a batch transfers its nested owning allocations cheaply. The destination
can retain the source allocation address, but the moved-from standard members
are only valid with unspecified state. Do not assume their strings or vectors
are empty; they can still be destroyed or assigned new values safely.

## How interviewers test this

**Prediction:** predict generated copy and move behavior from member types.

**Implementation:** implement only `relabel_copy`.

**Follow-up:** identify when a raw resource would invalidate Rule of Zero.

**Evidence:** show copy independence, destination address reuse, and safe source reassignment.

Expect to choose Rule of Zero over ceremonial special members and explain how
member types determine the enclosing type's copy and move behavior.

## Muscle memory

```cpp
struct FrameBatch {
    std::string source;
    std::vector<Frame> frames;
};
```

## The drills

Do not add special members to `FrameBatch`. Implement
`relabel_copy(FrameBatch, std::string)` by changing the local batch's label and
returning it. The assertions prove an independent copy, destination address
preservation after a move, and safe reassignment of the moved-from source.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/lessons/07_rule_of_zero -q
```

Continue to the [next lesson](../08_rule_of_five/).
