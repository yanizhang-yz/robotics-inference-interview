# 03 — Values and Copies

## Card

A C++ object is a **value by default**: pass it by value and the
function receives its own independent copy, not another name for the
caller's object.

```cpp
FrameMetadata with_frame_id(FrameMetadata metadata, int frame_id);
// `metadata` is the function's own object — edits stay in the copy
```

Update the parameter and return it: the caller gets a revised value
while the original stays untouched. The copy is complete — a
`std::string` member duplicates its text along with everything else.

## Predict

`main` runs `FrameMetadata updated = with_frame_id(original, 8);` where
`original.frame_id` is `7`. What are the two frame IDs afterward?

- A) `original` is `7`, `updated` is `8`
- B) Both are `8` — the function changed the caller's object
- C) Both are `7` — the edit was lost with the copy

<!-- predict
answer: A
why-A: Right — the function edits its own by-value copy and returns it, so the revision lands in `updated` and `original` is untouched.
why-B: A by-value parameter is an independent object; nothing done to it can reach the caller's `original`.
why-C: Returning the parameter hands the edited copy to the caller — the edit survives, it just never touches the original.
-->

## Drill

In `starter.cpp`, implement `with_frame_id` by assigning the new
`frame_id` to the by-value `metadata` parameter and returning it. The
caller's `original` must come back unchanged.

Manual check: `PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/03_values_and_copies -q`

## Takeaway

- Passing by value creates an independent function parameter.
- Returning the changed parameter hands the caller a revised value.
- `std::string` copies its text, so the revised metadata keeps its
  source.

## Deep dive

A C++ object is a value by default, and passing one by value constructs
an independent function parameter: `with_frame_id` receives its own
`FrameMetadata`, not another name for the caller's object. Any update to
that parameter changes only the function's copy, so the pattern here —
edit the copy, return it — produces a revised value for the caller while
the original stays exactly as it was. The copy is complete: a
`std::string` member duplicates its text along with everything else,
which is why the returned metadata keeps the same `source` while
receiving its revised frame ID.

The framing comes straight from a camera pipeline. A capture record
describes what the wrist camera actually saw, and traceability demands
that record never change. When a later processing stage needs the frame
metadata under a new ID, the pipeline makes a revised value for that
stage and leaves the original intact — derived work gets labeled without
rewriting the record of the original capture.

That discipline scales across a robot's software. Data moves between
stages constantly, and when each stage creates a revised result rather
than mutating shared state, the earlier records remain available for
logging, debugging, and replay. Values make the reasoning local: what a
function receives by value, it cannot use to alter its caller's world.
