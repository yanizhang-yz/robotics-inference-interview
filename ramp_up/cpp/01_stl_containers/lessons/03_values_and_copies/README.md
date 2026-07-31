# 03 — Values and Copies

## Problem

Produce revised frame metadata for downstream processing without mutating the
capture record that described the original camera frame.

## Mental model

A C++ object is a **value by default**. Passing an object by value constructs
an independent function parameter: `with_frame_id` receives its own
`FrameMetadata`, not another name for the caller's object. Updating that
parameter changes only the copy returned from the function.

Copying a `std::string` also duplicates its value. The returned metadata keeps
the same `source` text while receiving its revised frame ID.

## Application

A camera pipeline may retain the capture record for traceability while making a
revised metadata value for a later processing stage. This lets the application
label derived work without changing the record of what the wrist camera first
captured.

## Prediction

Before running the program, predict both frame IDs after the call. Does
`original.frame_id` stay `7`, and does `updated.frame_id` become `8`?

## Guided implementation

Implement `with_frame_id(FrameMetadata metadata, int frame_id)` in
`starter.cpp`. Update the by-value `metadata` parameter and return it. Leave
the caller's `original` value unchanged.

## Verification

Run the reference with:

```bash
.venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/03_values_and_copies -q
```

Run the learner starter with:

```bash
PRACTICE=1 .venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/03_values_and_copies -q
```

The shared runner compiles with C++20 and `-Wall -Wextra -Werror=return-type`. The
untouched starter is expected to fail because `updated.frame_id` remains `7`.

## Explain it

- Passing by value creates an independent function parameter.
- Returning the changed parameter produces a revised value for the caller.
- `std::string` copies its text, so the revised metadata keeps its source.

## Next connection

Values make it easier to reason about data that moves between robotics stages:
each stage can create a revised result while preserving the earlier record for
logging, debugging, or replay.
