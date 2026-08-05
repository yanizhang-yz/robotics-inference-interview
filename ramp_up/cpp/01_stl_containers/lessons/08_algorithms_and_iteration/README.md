# 08 — Algorithms and Iteration

## Card

A container holds values; an algorithm operates over a **range** described
by an iterator pair. They are separate tools.

```cpp
for (const CameraFrame& frame : frames) { /* borrowed read-only view */ }
std::sort(ids.begin(), ids.end());        // reorders only this range
```

Range-based `for` visits every element without manually advancing an
iterator; `const ... &` means no copy and no mutation. `std::sort`
reorders exactly the range it is handed — build selected IDs into their
own vector, and sorting them leaves the input frames untouched.

## Predict

`fresh_frame_ids` sorts the `ids` vector it built from the input. What
happens to the order of `frames`?

- A) `frames` is reordered too, since `ids` was built from it
- B) Nothing — `std::sort` reorders only the range `ids.begin()` to `ids.end()`
- C) The sort fails to compile because `frames` is `const`

<!-- predict
answer: B
why-A: `ids` owns copied int IDs in a separate vector; no link back to `frames` exists for the sort to follow.
why-B: Right — an algorithm touches exactly the iterator range it receives, and that range names elements of `ids`.
why-C: `frames` being `const` is irrelevant here: the iterators handed to `std::sort` come from the non-const `ids`.
-->

## Drill

In `starter.cpp`, implement `fresh_frame_ids`: iterate `frames` with
`const CameraFrame&`, append `frame_id` to a new `std::vector<int>` for
every frame whose `timestamp_ns` is at least `cutoff_ns`, then sort the
IDs with `std::sort(ids.begin(), ids.end())` and return them.

Manual check: `PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/08_algorithms_and_iteration -q`

## Takeaway

- Containers own values; algorithms operate on the iterator range
  `begin()` to `end()`.
- `const CameraFrame&` iteration reads each frame once with no copies and
  no mutation.
- Sorting the result vector leaves the input container in its original
  order.

## Deep dive

Containers and algorithms are separate tools that meet at an iterator
pair. The vector owns the `CameraFrame` records; `std::sort` never sees a
container at all — it receives `ids.begin()` and `ids.end()` and reorders
exactly that range. Range-based iteration is the reading half of the same
idea: `for (const CameraFrame& frame : frames)` visits every element
without manually advancing an iterator, and `const` plus `&` together say
each `frame` is a borrowed, read-only view of the existing element rather
than a fresh copy.

The drill mirrors a real pipeline decision. An inference pipeline may
receive camera frames in timing or transport order, but a downstream
batch builder can require a stable ID order. Selection and ordering are
therefore separate steps: first pick the frames fresh enough for the
batch, then sort only the compact list of IDs handed to the next stage.
Building the IDs into their own vector is what keeps the original capture
records untouched — still in arrival order for logging, diagnostics, or
another policy that needs their metadata.

That separation generalizes. Keep the input records whenever later stages
need them, and create a new ordered result whenever the next stage needs
a different representation. The `begin()`/`end()` convention is what lets
one generic algorithm reorder any range a container can describe.
