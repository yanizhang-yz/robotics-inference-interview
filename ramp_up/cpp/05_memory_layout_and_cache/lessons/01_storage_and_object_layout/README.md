# 01 — Storage and Object Layout

## The problem this lesson solves

A frame object has small bookkeeping members and a potentially large pixel
payload. Confusing the object's size with its payload size produces incorrect
memory budgets.

## The lesson

`Frame` contains `width`, `height`, and a `std::vector` object. The vector owns a
separate allocation whose `std::uint8_t` elements are contiguous. Therefore
`sizeof(Frame)` describes the members stored directly in the frame object, while
`frame.pixels.size() * sizeof(std::uint8_t)` describes its pixel payload.

Draw these as two regions: a `Frame` box containing dimensions and the vector's
handle state, with that vector referring to a second box of adjacent pixels. A
copy, move, or destruction operation acts on both through the vector's ownership
rules, but the two regions are still distinct.

## How interviewers test this

Expect to account for an image or tensor object's bytes, identify where each
region lives, and explain why `sizeof` does not recursively include dynamic
allocations.

## Muscle memory

```cpp
const std::size_t payload = frame.pixels.size() * sizeof(std::uint8_t);
```

Ask which bytes are inline members and which bytes belong to owned allocations.

## The drills

Implement `frame_payload_bytes` and check the vector's adjacency guarantee in
`pixels_are_contiguous`. The assertions distinguish the frame object from its
12-byte payload.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/05_memory_layout_and_cache/lessons/01_storage_and_object_layout -q
```
