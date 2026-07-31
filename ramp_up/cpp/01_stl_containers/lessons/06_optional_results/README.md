# 06 — Optional Results

## Problem

Find the most recent joint-position sample at or before an inference timestamp,
without pretending a sample always exists.

## Mental model

`std::optional<T>` is an **owned value that may be absent**. It either contains
a complete `T` or contains no value. Here, `std::optional<JointSample>` owns a
copy of the matching `JointSample`; it does not point into `samples`.

This differs from the raw pointer in the previous lesson. A pointer such as
`const JointSample*` is borrowed access to an object owned elsewhere, so its
validity depends on that owner's lifetime. An optional result is independent:
after `latest_at_or_before` returns, its `JointSample` remains available even
if the source vector is changed or destroyed.

Use `has_value()` to test whether a value exists. After that check,
`found->position` uses `operator->` to access a member of the contained
`JointSample`. `std::nullopt` is the explicit spelling for an empty optional.

## Application

Robot inference often aligns sensor streams by timestamp. Before running a
model for an image at time 25, a controller can ask for the newest joint state
that is no later than time 25. With samples at 10, 20, and 30 nanoseconds, the
aligned state is the sample at 20. For a timestamp before the first sample,
there is no aligned state, so the function returns `std::nullopt` and the
caller can wait, skip inference, or choose a fallback.

## Prediction

Before running the program, answer these questions:

1. What does `found.has_value()` report for a lookup at timestamp 25? What
   about a lookup at timestamp 5?
2. Why is `found->timestamp_ns` valid only after `found.has_value()` is true?
   What does `operator->` access in this expression?
3. What does `std::nullopt` represent, and when should this function return it?
4. Does the returned `JointSample` depend on the source vector remaining alive?
   Why is this answer different from a borrowed raw pointer?

## Guided implementation

In `starter.cpp`, create an empty `std::optional<JointSample> result`. Scan the
samples in order. Each sample whose timestamp is at or before the requested
timestamp becomes the new result. Return `result` after the loop; it stays
empty when no sample qualified.

## Verification

Run the reference with:

```bash
.venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/06_optional_results -q
```

Run the learner starter with:

```bash
PRACTICE=1 .venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/06_optional_results -q
```

The shared runner compiles with C++20 and `-Wall -Wextra -pedantic`. The
untouched starter is expected to fail because the lookup at timestamp 25 has
no value.

## Explain it

- `std::optional<JointSample>` communicates “a sample, if one exists.”
- `std::nullopt` communicates an intentionally absent result.
- `has_value()` proves a value is present before `operator->` reads it.
- The returned sample is owned by the optional, not borrowed from the vector.

## Next connection

Use an optional value when a computation may legitimately have no result and
the caller should receive an independent value. Use a raw pointer only when an
API needs nullable, non-owning access to an object someone else owns.
