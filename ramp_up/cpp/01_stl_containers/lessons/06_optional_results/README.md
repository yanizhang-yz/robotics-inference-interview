# 06 — Optional Results

This lesson gives you the second way to say "maybe no answer":
`std::optional<T>`, a value that *owns* its contents — the self-contained
alternative to lesson 05's borrowed pointer. After it, you can build,
check, and read optionals, choose between `T*` and `optional<T>` on
purpose, and write the drill: a timestamp-alignment lookup that returns
the latest joint sample at or before a query time, or nothing. Every
output shown in a comment was run and verified.

## The problem this lesson solves

Lesson 05's `nullptr` answer works, but the pointer it returns has a
string attached: it borrows, so it is only valid while the source
container lives, unmoved. Sometimes you want the maybe-answer to stand on
its own — computed, then kept, while the source buffer keeps churning. A
pointer also cannot express "maybe" for a *computed* value: an average or
an interpolated sample has no owner to borrow from. The pre-modern
workarounds — return a sentinel like `-1.0`, or a `bool` plus an
out-parameter — either poison arithmetic or split one answer across two
places. Python solves this by returning the value or `None`;
`std::optional<T>` is that idea with the type system enforcing the
"or nothing" part.

## The lesson

### A box that may be empty

**`std::optional<T>`** (from `<optional>`) is a value type that either
contains a complete `T` or contains nothing. Freshly declared, it is
empty; assigning a `T` fills it. **`has_value()`** asks which state it is
in:

```cpp
std::optional<double> reading;   // starts EMPTY — no double inside
reading.has_value()              // -> false
reading = 30.0;                  // assignment fills the box
reading.has_value()              // -> true
```

The explicit spelling for "empty" is **`std::nullopt`** — writing
`return std::nullopt;` is the C++ for Python's `return None`, and
`std::optional<double> none = std::nullopt;` declares the empty state on
purpose rather than by omission.

### Reading the box — deliberately pointer-shaped

Access reuses lesson 05's spellings: `*opt` is the contained value,
`opt->member` reaches a member of it, and an optional in a boolean context
is truthy exactly when full:

```cpp
*reading                 // -> 30.0    the contained value
if (reading) { ... }     // body ran — a full optional is truthy
std::optional<double> none = std::nullopt;
if (!none) { ... }       // body ran — an empty one is falsy
```

Same syntax, same discipline: **check, then read**. Reading `*opt` or
`opt->` on an *empty* optional is undefined behavior, just like following
`nullptr`. Two safer accessors exist. **`value_or(fallback)`** collapses
check-and-read into one call:

```cpp
none.value_or(0.0)       // -> 0.0     empty: you get the fallback
reading.value_or(0.0)    // -> 30.0    full: you get the contents
```

And **`value()`** reads with a seatbelt — on an empty optional it throws
instead of corrupting anything. Verified, uncaught, it ends the program
like this:

```text
libc++abi: terminating due to uncaught exception of type
    std::bad_optional_access: bad_optional_access
```

Loud beats silent, but the idiomatic shape remains `if (opt)` followed by
`*opt` or `opt->`.

### Owned, not borrowed — the contrast with lesson 05

Here is the reason this lesson exists. `result = sample;` inside a loop
*copies* the sample into the optional (lesson 03 doing quiet good work),
so the answer is independent of its source. Destroy the source and the
answer survives:

```cpp
auto found = latest_at_or_before(samples, 25);
samples.clear();          // the source data is gone
found->timestamp_ns       // -> 20   still valid — the optional owns its copy
```

Do that with lesson 05's borrowed pointer and you have a dangling pointer.
The decision between them is now one question — *who owns the answer?*

|                    | `const T*` (lesson 05)  | `std::optional<T>`      |
|--------------------|-------------------------|-------------------------|
| absent is spelled  | `nullptr`               | `std::nullopt`          |
| present means      | borrow of owner's object| a copy the optional owns|
| stays valid until  | owner moves or dies     | the optional itself dies|
| cost when present  | free                    | one copy of `T`         |

Borrow when the caller just peeks and the owner outlives the peek; own
when the answer must outlive or detach from its source. The drill wants
detachment: an aligned sample survives however the telemetry buffer churns
afterward.

### The running-best scan with an optional accumulator

The drill's algorithm is a running best — like tracking a maximum — with
one classic wrinkle: what do you initialize the best to, before any
candidate qualifies? Sentinels (`-1`, `INT_MIN`) are exactly the hack this
type retires. An empty optional *is* "no best yet":

```cpp
std::optional<JointSample> result;              // no candidate yet
for (const JointSample& sample : samples) {
    if (sample.timestamp_ns <= timestamp_ns &&
        (!result.has_value() || sample.timestamp_ns > result->timestamp_ns)) {
        result = sample;                        // copy the new best into the box
    }
}
return result;             // full if anything qualified, nullopt otherwise
```

One subtlety worth naming: in `!result.has_value() || sample... >
result->timestamp_ns`, the `||` **short-circuits** — it evaluates left to
right and stops at the first `true`, so `result->timestamp_ns` only runs
when a value is present. The check-then-read discipline, packed into one
condition.

## Muscle memory

Type these until they come out without thinking:

```cpp
#include <optional>
std::optional<T> r;           // starts empty — "no answer yet"
r = value;                    // fills it (copies the value in)
if (r) { use(*r); }           // check, THEN read
r->member                     // member of the contained value
r.value_or(fallback)          // read with a default, no check needed
return std::nullopt;          // "no result" — Python's return None
!r.has_value() || r->x > y    // short-circuit guard inside one condition
```

## The drills

One function in `starter.cpp`; `main` checks a hit, a miss, and an
out-of-order input.

### `latest_at_or_before(samples, timestamp_ns)`

Return the sample with the greatest timestamp that is `<= timestamp_ns`,
or `std::nullopt` if every sample is later than the query.

```cpp
const std::vector<JointSample> samples{{10, 0.1}, {20, 0.2}, {30, 0.3}};
latest_at_or_before(samples, 25)->timestamp_ns    // -> 20
latest_at_or_before(samples, 25)->position        // -> 0.2
latest_at_or_before(samples, 5).has_value()       // -> false   all samples too late
```

The shape is the running-best scan from the lesson, verbatim. The trap the
tests check: the second input is `{{20, 0.2}, {10, 0.1}}` — *out of
order* — and the answer must still be the sample at 20. So "latest" means
greatest timestamp, not last-seen: no early `break` on the first
qualifying sample, no assuming the vector is sorted; compare each
candidate against the current best. Boundary note, also verified: the
comparison is `<=`, so a sample exactly at the query time qualifies.

**Where you'll see it:** in interviews this is the "as-of" lookup —
"Time Based Key-Value Store" (LeetCode 981) is literally
`get(key, timestamp)` returning the latest value at or before a time — and
the empty-accumulator scan replaces every awkward
`best = -1 // means unset` you have ever written in a running-max problem.
In robotics it is stream alignment, a fixture of every sensor-fusion and
learned-control stack: before running a policy on an image exposed at
t=25, fetch the newest joint state at or before t=25 so the model sees a
consistent world; if none exists yet, `nullopt` tells the caller to wait
or skip the cycle rather than fabricate a state. Inference systems reuse
the shape for checkpoint selection ("newest checkpoint at or before step
N") and feature stores ("value of this feature as of time T"). And
`std::optional` itself is the standard return type for parse-and-lookup
APIs across modern C++ — you will read it in headers daily.

## How to practice

Write your attempt in `starter.cpp`, then run the tests against it:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/06_optional_results -q
```

Drop the `PRACTICE=1` to run them against the reference `solution.cpp`.
The untouched starter returns `std::nullopt` unconditionally and fails at
`assert(found.has_value())` — and note how `main` itself obeys the
check-then-read order before ever touching `found->timestamp_ns`.
