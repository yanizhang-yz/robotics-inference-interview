# 04 — Traversal and Contiguous Storage

## The problem this lesson solves

A row-first loop and a column-first loop can visit exactly the same matrix
elements while presenting very different address sequences to the memory
system.

## The lesson

A flat row-major buffer stores element `(row, col)` at `row * cols + col`.
Walking rows outside and columns inside produces offsets `0, 1, 2, ...`.
Interchanging the loops keeps the same indexing formula but jumps by `cols`
elements in the inner loop.

For a 2-by-3 buffer, the two literal orders are `{0,1,2,3,4,5}` and
`{0,3,1,4,2,5}`. Both visit six elements exactly once. The operation count and
Big-O are unchanged; only the address order, and therefore likely locality,
changes. The function name `column_major_offsets` describes column-first
traversal here, not a change to the buffer's row-major layout.

## How interviewers test this

**Prediction:** predict row-first and column-first offset sequences.

**Implementation:** implement both traversal traces.

**Follow-up:** explain why equal results can have different locality.

**Evidence:** show identical elements and different address order.

Expect to flatten two-dimensional indices, trace small examples by hand, and
interchange loops to match a stated storage order.

## Muscle memory

```cpp
const std::size_t offset = row * cols + col;
```

For row-major data, make `col` the rapidly changing loop variable when the
algorithm permits it.

## The drills

Generate both access-order vectors for arbitrary row and column counts. Keep the
same row-major offset formula in both functions and change only loop nesting.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/05_memory_layout_and_cache/lessons/04_traversal_and_contiguous_storage -q
```

Continue to the [next lesson](../05_aos_and_soa/).
