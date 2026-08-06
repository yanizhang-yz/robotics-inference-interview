# 03 — Cache Lines and Locality

## The problem this lesson solves

Two loops can perform the same number of loads yet make very different use of
the memory chunks fetched for those loads.

## The lesson

Memory is cached in line-sized chunks. This lesson uses a deliberately simple
address model: the first element begins at byte offset zero, element `i` begins
at `i * stride_elements * element_bytes`, and integer division by `line_bytes`
gives its modeled line index.

With the required four-byte `float` representation and a 64-byte model line, 16
contiguous floats occupy one line. A stride of 16 floats moves each access to a
new line, so 16 accesses touch 16 line indices. This arithmetic describes useful
bytes per modeled line; it is not a benchmark and does not claim a particular
machine has a 64-byte hardware cache line.

## How interviewers test this

**Prediction:** predict literal modeled line indices for packed and strided access.

**Implementation:** implement the line-count calculation.

**Follow-up:** explain what the model omits about real caches.

**Evidence:** enumerate touched lines from addresses, not timing.

Expect to translate element strides into byte strides, enumerate line indices,
and explain spatial locality without promising a hardware-specific speedup.

## Muscle memory

```cpp
const std::size_t line = byte_offset / line_bytes;
```

Count distinct line indices, then ask how much of each line the algorithm uses.

## The drills

Implement `cache_lines_touched` by inserting the modeled line index for every
access into a set. The fixtures are tiny; do not allocate a benchmark-sized
buffer or measure elapsed time.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/05_memory_layout_and_cache/lessons/03_cache_lines_and_locality -q
```

Continue to the [next lesson](../04_traversal_and_contiguous_storage/).
