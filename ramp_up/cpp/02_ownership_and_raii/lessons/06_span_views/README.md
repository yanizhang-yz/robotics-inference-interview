# 06 — Span Views

This lesson uses `std::span` to pass a tensor range without copying it or giving
away ownership.

## The problem this lesson solves

A raw pointer alone does not carry a length, while a `std::vector&` unnecessarily
requires one container type. Tensor kernels need a safe range-shaped interface
over contiguous storage owned elsewhere.

## The lesson

`std::span<T>` is a non-owning view: pointer plus element count. A mutable
`span<float>` permits edits to the owner's elements; `span<const float>` permits
only reads. `subspan(offset, count)` creates another view, not another allocation,
so scaling a middle view changes only those original vector elements.

A span does not extend storage lifetime. Its owner must remain alive, and
operations that relocate the owner's storage invalidate the view. Never return
a span into a local vector that is about to die.

## How interviewers test this

Expect to select mutable versus const span, handle an empty range, and state the
owner/lifetime and reallocation conditions that keep a view valid.

## Muscle memory

```cpp
std::span<float> writable = tensor;
std::span<const float> readable = tensor;
auto middle = writable.subspan(1, 3);
```

A view can outlive neither its storage nor that storage's address stability.

## The drills

Implement `scale_in_place(std::span<float>, float)` and
`mean(std::span<const float>)`. Scale only the supplied middle subspan, average
the full result, and define the empty mean as `0.0`.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/02_ownership_and_raii/lessons/06_span_views -q
```

Drop `PRACTICE=1` to check the reference solution.
