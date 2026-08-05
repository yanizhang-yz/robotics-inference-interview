# 06 — Optional Results

## Card

`std::optional<T>` is an **owned value that may be absent** — it
contains a complete `T` or no value at all.

```cpp
std::optional<JointSample> latest_at_or_before(
    const std::vector<JointSample>& samples, std::int64_t timestamp_ns);
```

The result owns a copy of the matching sample; it does not point into
`samples`. `has_value()` tests presence, `found->position` then reads
the contained value, and `std::nullopt` spells "empty" on purpose.
Unlike a borrowed pointer, this result outlives the source vector.

## Predict

`latest_at_or_before` returns a populated optional, then the source
vector is destroyed. Is the sample inside the optional still safe to
read?

- A) Yes — the optional owns its own copy of the sample
- B) No — like a borrowed pointer, it referred into the vector's storage
- C) Only after calling `has_value()` again to re-validate it

<!-- predict
answer: A
why-A: Right — the optional contains a complete `JointSample` by value, so its lifetime is independent of the vector's.
why-B: That is the raw-pointer contract from the previous lesson; an optional stores the `T` inside itself instead of pointing elsewhere.
why-C: `has_value()` reports whether the optional is populated — it has no link to the vector to re-validate.
-->

## Drill

In `starter.cpp`, implement `latest_at_or_before`: start from an empty
`std::optional<JointSample> result`, scan every sample, and replace
`result` whenever a sample is at or before the requested timestamp and
newer than the current result (or no result exists yet). Samples may be
out of order — "latest" means greatest timestamp, not last position.

Manual check: `PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/06_optional_results -q`

## Takeaway

- `std::optional<JointSample>` communicates "a sample, if one exists";
  `std::nullopt` is the intentional absence.
- `has_value()` proves a value is present before `operator->` reads it.
- The returned sample is owned by the optional, not borrowed from the
  vector.

## Deep dive

`std::optional<T>` either contains a complete `T` or contains no value.
Here `std::optional<JointSample>` owns a copy of the matching sample
rather than pointing into `samples`, which is the deliberate contrast
with the previous lesson: a `const JointSample*` would be borrowed
access whose validity depends on the vector's lifetime, while an
optional result is independent — after `latest_at_or_before` returns,
its sample remains available even if the source vector is changed or
destroyed. `has_value()` tests whether a value exists; only after that
check does `found->position` use `operator->` to reach a member of the
contained sample, and `std::nullopt` is the explicit spelling for the
empty case. The input need not be sorted: "latest" means the qualifying
sample with the greatest timestamp, wherever it appears in the vector.

The scenario is stream alignment, a constant of robot inference. Before
running a model on an image captured at time 25, a controller asks for
the newest joint state no later than 25; with samples at 10, 20, and 30
nanoseconds, in any order, the aligned state is the sample at 20. For a
timestamp earlier than every sample there is no aligned state, so the
function returns `std::nullopt` and the caller decides — wait, skip
inference, or choose a fallback. Use an optional when a computation may
legitimately have no result and the caller should receive an independent
value; keep raw pointers for nullable, non-owning access to objects
owned elsewhere.
