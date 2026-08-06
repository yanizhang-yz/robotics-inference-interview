# 06 — Allocation and `reserve`

## The problem this lesson solves

Repeated `push_back` operations can move a growing batch to larger allocations,
creating latency spikes and invalidating pointers, references, and iterators into
the old allocation.

## The lesson

A vector exposes `size()` for constructed elements and `capacity()` for the
number it can hold before another allocation is required. When growth exceeds
capacity, the vector allocates a larger block and moves or copies its elements.
That reallocation invalidates borrows into the former block.

`reserve(n)` requests capacity for at least `n` elements without constructing
them. Starting from an empty vector and reserving a positive final count produces
one observable capacity change; the following `n` pushes fit without another.
Without reserve, the standard library chooses its own growth policy. The
unreserved change count is printed as an observation, not asserted. The
portable claim is that `reserve(100)` causes one initial capacity change and
the following 100 pushes cause no additional change.

Both builds still have amortized linear total work. Reserving removes predictable
growth events from the middle of this construction path; it does not make every
allocation free or guarantee a timing ratio.

## How interviewers test this

**Prediction:** predict capacity changes and borrow invalidation with reserve.

**Implementation:** implement the capacity-change counter.

**Follow-up:** explain why unreserved growth count is implementation-defined.

**Evidence:** prove one reserved change and treat the unreserved count as observation.

Expect to distinguish size from capacity, state exactly when reallocation
invalidates borrows, and use `reserve` when a credible upper bound is known.

## Muscle memory

```cpp
std::vector<int> batch;
batch.reserve(expected_items);
```

Reserve for a known count; use `resize` only when those elements should already
exist.

## The drills

Track capacity from an empty vector through optional reserve and every push.
Assert the reserved construction path and report the implementation-defined
unreserved count for comparison.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/05_memory_layout_and_cache/lessons/06_allocation_and_reserve -q
```

Continue to the [module capstone](../../).
