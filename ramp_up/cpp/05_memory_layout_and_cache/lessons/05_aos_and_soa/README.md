# 05 — AoS and SoA

## The problem this lesson solves

An inference post-processing pass needs only detection scores and winning IDs,
but a record-oriented layout places box coordinates in the same scan path.

## The lesson

An array of structs (AoS) stores complete `Detection` records together. This is
convenient when each operation consumes most fields of one detection. A struct
of arrays (SoA) stores IDs and scores in separate, equal-length columns. That
shape makes a score-only scan contiguous and provides packed inputs for possible
SIMD processing.

`best_detection_soa` asserts that `ids` and `scores` have equal lengths before
using a score index in the ID column. Both layouts return `std::optional<int>` so
an empty frame has no magic ID.

The lesson calculates AoS scan exposure as
`N * sizeof(Detection)` and score-column exposure as `N * sizeof(float)`. This is
a theoretical comparison model for fields presented by each layout, not a claim
that the processor literally loads every counted byte. Real transfers depend on
cache lines, alignment, compiler output, and prior cache state.

## How interviewers test this

**Prediction:** predict which bytes each score scan consumes.

**Implementation:** implement equivalent AoS and SoA queries.

**Follow-up:** explain column-length invariants and update trade-offs.

**Evidence:** show equal answers while naming the fields touched.

Expect to choose a layout from the fields that travel together, preserve column
invariants, and discuss cache/SIMD implications without inventing timing ratios.

## Muscle memory

```cpp
assert(columns.ids.size() == columns.scores.size());
```

Use AoS for record-centric work and consider SoA for repeated scans over a small
subset of fields.

## The drills

Implement the same best-detection query for both layouts. Return an empty
optional for empty input and keep the SoA columns synchronized.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/05_memory_layout_and_cache/lessons/05_aos_and_soa -q
```

Continue to the [next lesson](../06_allocation_and_reserve/).
