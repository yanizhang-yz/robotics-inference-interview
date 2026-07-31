# 08 — Algorithms and Iteration

## Problem

A camera adapter has recorded frames in arrival order. Select the IDs of frames
whose timestamps are at or after a cutoff, then return those IDs in ascending
numeric order. For frames `{3, 30}`, `{1, 10}`, and `{2, 20}`, a cutoff of
`20` produces `{2, 3}`.

## Mental model

A container holds values. An algorithm operates over a range of those values.
They are separate tools: the vector owns the `CameraFrame` values, while
`std::sort` receives the vector's iterator pair, `ids.begin()` and `ids.end()`,
to describe the range it should reorder.

Range-based iteration visits every element in a container without manually
advancing an iterator. Write `for (const CameraFrame& frame : frames)` when
you only need to inspect each frame. `const` prevents accidental changes, and
`&` means `frame` is a borrowed view of the existing element rather than a new
copy. The loop builds a separate vector of selected IDs, so its later sort does
not change `frames`.

## Application

An inference pipeline may receive camera frames in timing or transport order,
but a downstream batch builder can require a stable ID order. First select the
frames fresh enough for the batch. Then sort only the compact list of IDs that
will be handed to the next stage. This keeps the original capture records
available in their original order for logging, diagnostics, or another policy.

`fresh_frame_ids` takes a read-only frame collection and a timestamp cutoff. It
returns a new collection containing only qualifying IDs, sorted in ascending
order.

## Prediction checkpoints

Before running the program, answer these questions:

1. What is the input order of the frame IDs in `frames`: `3, 1, 2` or `1, 2, 3`?
2. With cutoff `20`, which frames pass the timestamp check, and in what order
   are their IDs appended before sorting?
3. What is the final result order after `std::sort(ids.begin(), ids.end())`?
4. Does sorting `ids` change the order of the original `frames` vector? Why or
   why not?
5. Why is `const CameraFrame&` a good loop-variable type here instead of a
   copied `CameraFrame`?

## Guided implementation

In `starter.cpp`, create an empty `std::vector<int>` named `ids`. Iterate over
`frames` using `const CameraFrame&`. For every frame whose `timestamp_ns` is at
least `cutoff_ns`, append `frame.frame_id` to `ids`. Finally, call
`std::sort(ids.begin(), ids.end())` and return `ids`.

## Verification

Run the reference with:

```bash
.venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/08_algorithms_and_iteration -q
```

Run the learner starter with:

```bash
PRACTICE=1 .venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/08_algorithms_and_iteration -q
```

The shared runner compiles with C++20 and `-Wall -Wextra -Werror=return-type`. The
untouched starter is expected to fail for cutoff `20` because it returns no
selected IDs.

## Explain it

- The frame vector owns the input records; the ID vector owns a new result.
- Range-based iteration reads each frame once, and `const T&` avoids copies and
  mutation.
- `begin()` and `end()` specify the range for a generic algorithm such as sort.
- Sorting the result container leaves the original input container unchanged.

## Next connection

Selection and ordering are separate pipeline decisions. Keep the input records
when later stages need their metadata, and create a new ordered result whenever
the next stage needs a different representation.
