# 08 — Algorithms and Iteration

This lesson separates two ideas Python fused into one: containers, which own
elements, and algorithms, which operate on a *range* of positions handed to
them. After it, you can walk a vector of timestamped camera-frame records
with a range-for, select the fresh ones, and sort the result with
`std::sort` — knowing precisely why the sort cannot disturb the source data.
That pipeline is the drill: `fresh_frame_ids`. Every output shown in a
comment was run and verified.

## The problem this lesson solves

Coming from `sorted(...)` and list comprehensions, C++'s sorting tool has a
contract you have to actually learn: `std::sort` mutates, returns nothing,
and operates on a pair of positions rather than "the list". Not knowing that
contract produces two opposite failures. One is hand-rolling a selection sort
under interview pressure because you never learned the library call. The
other is superstition — hesitating to sort a list of IDs *derived from* a
frame vector for fear the frames get reordered too. One mental model
dissolves both: an algorithm can only touch the range you hand it.

## The lesson

### A record type for the drill

The starter bundles two facts per camera frame into a struct, the same tool
lesson 04 used for `JointState`. **Brace initialization** fills the fields in
declaration order:

```cpp
struct CameraFrame {
    int frame_id;
    std::int64_t timestamp_ns;
};

CameraFrame f{3, 30};   // fields in declaration order
f.frame_id              // -> 3
f.timestamp_ns          // -> 30
```

**`std::int64_t`** is an integer of exactly 64 bits. Timestamps here are
nanoseconds, and a 32-bit int overflows after about 2.1 seconds of
nanoseconds — robots time everything in this type, so get used to seeing it.

### Iterators: positions, and the half-open range

An **iterator** is an object that names a *position* inside a container.
`v.begin()` names the first element's position; `v.end()` names one *past*
the last — the same one-past-the-end idea you met in lesson 07 as
`data() + size()`, generalized to every container. A `(begin, end)` pair
describes a **range**: **half-open**, begin included, end excluded — exactly
the convention of Python's `lst[start:stop]` slices, so your instincts
transfer. Because `end` is excluded, an empty container simply has
`begin() == end()`, and algorithms handle it with zero special-casing. The
pair convention is also what makes the library composable: `std::sort`,
`std::find`, `std::count`, and `std::max_element` (the library form of
lesson 04's max reduction) all take the same two positions — learn one
signature and you have learned the family's.

### `std::sort`: in place, ascending, returns nothing

From `<algorithm>`:

```cpp
std::vector<int> ids{3, 1, 2};
std::sort(ids.begin(), ids.end());
// ids -> {1, 2, 3}    reordered in place
```

Python gave you two verbs: `sorted(lst)` (returns a sorted copy) and
`lst.sort()` (in place, returns `None`). `std::sort` is the `lst.sort()`
verb, and there is no copying twin — if you need the original order, copy
first. Remember the Python trap where `result = lst.sort()` stored `None`
and crashed later? C++ upgrades that mistake into a compile-time stop,
because `std::sort` returns no value at all:

```cpp
auto r = std::sort(ids.begin(), ids.end());
```

```text
sortvoid.cpp:6:10: error: variable has incomplete type 'void'
```

One more Python difference worth filing away: Python's sort is *stable* —
equal elements keep their original relative order. `std::sort` makes no such
promise; when you need Python's guarantee, the library spells it
`std::stable_sort`.

### The algorithm cannot touch what you don't hand it

`std::sort` never sees a container — only two positions. So sorting a vector
you *built from* another container cannot reorder the source: no link back
exists for the sort to follow. The compiler also polices the reverse
direction: a `const` vector hands out read-only positions, and the sort's
first attempted write is rejected. The error erupts from deep inside the
library:

```cpp
const std::vector<int> ids{3, 1, 2};
std::sort(ids.begin(), ids.end());
```

```text
sift_down.h:61:14: error: read-only variable is not assignable
sortconst.cpp:6:10: note: in instantiation of function template specialization
      'std::sort<std::__wrap_iter<const int *>>' requested here
```

New reading skill: the `error:` names a library file because `sort` is
stamped out for your exact element type (lesson 09 explains that machinery);
scan down for the `note: ... requested here` that names *your* file and line
— that is where to fix. One more contract detail: the elements must support
`<`. Ints do; sorting `CameraFrame` records directly would need a comparison
rule you have not learned yet — which is exactly why the drill sorts the IDs,
not the frames.

### Filter, then sort: two steps, two containers

In Python the drill would be one line:
`sorted(f.frame_id for f in frames if f.timestamp_ns >= cutoff)`. C++ spells
out the same three moves:

```cpp
const std::vector<CameraFrame> frames{{3, 30}, {1, 10}, {2, 20}};

std::vector<int> ids;                        // 1. a NEW result vector
for (const CameraFrame& frame : frames) {    // 2. select — const&: no copy, no touch
    if (frame.timestamp_ns >= 20) {
        ids.push_back(frame.frame_id);
    }
}
std::sort(ids.begin(), ids.end());           // 3. order the RESULT
// ids                -> {2, 3}
// frames[0].frame_id -> 3    input untouched, still in arrival order
```

The loop header is lesson 04's parameter convention applied per element:
`const CameraFrame&` aliases each record read-only instead of copying a
16-byte struct every iteration. Last tool, used by the starter's asserts —
vectors compare element-wise with plain `==`:

```cpp
ids == std::vector<int>{2, 3}    // -> true
```

## How interviewers test this

- **Prediction probe:** Trace half-open ranges and the mutation produced by sorting.
- **Implementation probe:** Filter then sort frame IDs in `fresh_frame_ids`.
- **Follow-up probe:** Give the complexity of the operations and explain comparator requirements.

## Muscle memory

Type these until they come out without thinking:

```cpp
for (const Frame& f : frames)         // read a vector of structs: const&, no copies
std::sort(v.begin(), v.end());        // in place, ascending, returns void
std::vector<int> out;                 // build results into a NEW vector...
out.push_back(x);                     // ...one selected element at a time
out == std::vector<int>{1, 2}         // element-wise equality
// begin() included, end() excluded — Python's [start:stop] convention
```

## The drills

One function in `starter.cpp`; it composes everything above.

### `fresh_frame_ids(frames, cutoff_ns)`

Return the IDs of every frame captured at or after `cutoff_ns`, sorted
ascending.

```cpp
const std::vector<CameraFrame> frames{{3, 30}, {1, 10}, {2, 20}};
fresh_frame_ids(frames, 20)   // -> {2, 3}   frame 1 (t=10) is stale; IDs sorted
fresh_frame_ids(frames, 40)   // -> {}       nothing fresh enough: empty vector
```

The shape is the filter-then-sort pattern verbatim: an empty
`std::vector<int>`, one `const CameraFrame&` loop with the timestamp test,
`std::sort` on the result, return it. Two traps the tests check. The cutoff
is *inclusive* — `frame.timestamp_ns >= cutoff_ns`, so a frame stamped
exactly at the cutoff counts; write `>` and frame 2 vanishes from the first
case above. And the empty result must fall through cleanly — sorting a range
where `begin() == end()` is a legal no-op, so no special case is needed.
(You could not reorder `frames` even by accident: it arrives `const`.)

**Where you'll see it:** select-then-sort opens a whole interview genre —
"Merge Intervals" collects then sorts by start, "Meeting Rooms" sorts times,
and every top-k-after-filter variant starts here; `std::sort` is the single
most-typed algorithm call in interview C++. In a robot's inference pipeline
this exact function is stale-frame rejection: transport can deliver camera
frames out of order, so before building a batch you keep only frames newer
than the cutoff and hand downstream a deterministic ID-ordered list — while
the arrival-ordered originals stay intact for logging and diagnostics.

## How to practice

Write your attempt in `starter.cpp`, then run the tests against it:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/08_algorithms_and_iteration -q
```

Drop the `PRACTICE=1` to run them against the reference `solution.cpp`.
