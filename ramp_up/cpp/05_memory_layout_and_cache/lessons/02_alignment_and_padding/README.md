# 02 — Alignment and Padding

## The problem this lesson solves

A record can consume substantially more space than the sum of its fields. In a
large detection or telemetry array, repeated padding reduces record density.

## The lesson

Each field begins at an offset compatible with that field's alignment. The
compiler preserves declaration order and inserts padding between fields or at
the record's tail; it does not silently reorder your data.

On the curriculum's target ABI, `BadOrder` alternates small and aligned fields,
while `GoodOrder` groups the `double`, then the `int`, then the two `char` fields.
Both records represent identical values and both satisfy the `double` alignment,
but the grouped order needs fewer padding bytes. Use `sizeof` to verify a paper
offset calculation, not as a substitute for doing it.

## How interviewers test this

**Prediction:** calculate member offsets, padding, and total size.

**Implementation:** implement the denser field order.

**Follow-up:** explain ABI dependence without assuming packing.

**Evidence:** justify every offset from alignment constraints.

Expect to calculate member offsets, explain internal and tail padding, reorder a
record without dropping fields, and connect the smaller stride to more records
per cache line.

## Muscle memory

```cpp
struct DenseRecord {
    double timestamp;
    int id;
    char valid;
    char source;
};
```

Start with the most strictly aligned fields, while preserving semantic or ABI
constraints when those constraints matter.

## The drills

Reorder `GoodOrder`, update its initializer, and return both compiler-selected
sizes from `padded_sizes`. The same field values must survive the layout change.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/05_memory_layout_and_cache/lessons/02_alignment_and_padding -q
```

Continue to the [next lesson](../03_cache_lines_and_locality/).
