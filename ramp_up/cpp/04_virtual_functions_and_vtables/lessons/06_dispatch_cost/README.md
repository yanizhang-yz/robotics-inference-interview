# 06 — Dispatch Cost

## The problem this lesson solves

An inference pipeline needs runtime-selectable backends without putting an
indirect virtual call around every element in a hot inner loop.

## The lesson

A virtual call adds a small indirect-dispatch cost and can prevent inlining
across that boundary. The useful design question is therefore where the boundary
belongs, not an ungrounded claim about nanoseconds.

`run_batch` makes one virtual call with a `std::span<const int>`. The selected
backend performs the whole loop internally. The counting backend makes this
shape observable: `batch_calls` becomes one, the sum is correct, and
`item_calls` remains zero. No timing benchmark is needed.

## How interviewers test this

**Prediction:** predict virtual call counts for per-item versus per-batch dispatch.

**Implementation:** implement the coarse batch boundary.

**Follow-up:** explain inlining and measurement trade-offs.

**Evidence:** use call counters and equal outputs rather than timing claims.

Expect to discuss dispatch overhead without exaggerating it, then move the
runtime boundary around coarse work such as a frame, tensor, or batch.

## Muscle memory

```cpp
return backend.infer_batch(inputs);  // one virtual call for the whole span
```

Choose the backend dynamically; keep tight per-item work behind that choice.

## The drills

Implement the counting backend and `run_batch`. Sum all inputs inside one
`infer_batch` call and leave the per-item call counter untouched.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables/lessons/06_dispatch_cost -q
```

Continue to the [module capstone](../../).
