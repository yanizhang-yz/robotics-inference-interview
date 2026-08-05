# 03 — Values and Copies

This lesson teaches the single deepest Python-to-C++ flip: in C++,
assignment and parameter passing **copy** by default. After it, you can
predict — for any assignment, any call — whether two names share one object
or hold two independent ones, and you can write the *revised-value* pattern:
edit a private copy, return it, leave the original untouched. The drill
applies that pattern to camera-frame metadata: derive a relabeled record
without altering the original capture record. Every output shown in a
comment was run and verified.

## The problem this lesson solves

Your Python instincts say that handing an object to a function shares it:
`v = u` aliases, mutation travels, and copying is something you ask for
with `list(u)`. Carry that instinct into C++ and you write a function that
"fixes" its argument, watch it compile cleanly, run cleanly — and change
nothing, because every edit landed on a private copy that died at the
closing brace. There is no error and no warning; the bug is invisible until
an assertion fires somewhere else. The cure is one rule, applied until it
is reflex: in C++ *you get a copy unless you say otherwise*, and this
lesson is the copy half of that rule (lesson 04 is the say-otherwise half).

## The lesson

### Assignment copies — the Python flip

In Python, `v = u` makes `v` a second name for the same list. In C++, the
same line builds a complete, independent second object:

```cpp
std::vector<int> u{1, 2};
std::vector<int> v = u;   // a full COPY — not an alias
v.push_back(3);
u.size()                  // -> 2   u never saw the append
v.size()                  // -> 3
```

Run the Python version of this and `u` ends up `[1, 2, 3]`; in C++ it
stays `{1, 2}`. Same two behaviors in both languages — alias and copy —
but opposite defaults: Python aliases and copies on request, C++ copies
and aliases on request. Most confusion between the two languages is
exactly this flip.

### Copies go all the way down

What does "copy" copy? A **member-wise copy**: each field of the object is
copied in turn, and each field uses its *own* copying behavior — a
`std::string` (C++'s owning string type) duplicates its text. Take the
drill's struct:

```cpp
struct FrameMetadata {
    int frame_id;
    std::string source;
};

FrameMetadata a{7, "wrist-camera"};
FrameMetadata b = a;         // copies the int AND the string
b.frame_id = 8;
a.frame_id                   // -> 7               untouched
b.source = "sim-camera";
a.source                     // -> "wrist-camera"  the text was duplicated too
```

Contrast with the Python ramp: `list(u)` there was a *shallow* copy —
new outer list, shared inner objects. A C++ struct copy has no shared
tail to worry about here: every member that owns data copies that data.
(The exception is a member that is itself a non-owning handle — that story
starts in lesson 05.)

### Parameters are copies too — the silent no-op

A plain `T` parameter is initialized by the same copy. The function gets
its own object; the caller's is out of reach:

```cpp
void relabel(FrameMetadata metadata) {   // by value: a private copy
    metadata.frame_id = 99;              // edits the copy...
}

FrameMetadata a{7, "wrist-camera"};
relabel(a);
a.frame_id    // -> 7   ...which is destroyed at return. Nothing happened.
```

This is the trap named in the problem statement, and it deserves a bold
sign: **mutating a by-value parameter is a silent no-op for the caller.**
No diagnostic exists because the code is legal — sometimes a private
scratch copy is exactly what you want. Which is the flip side worth
savoring: a by-value function *cannot* corrupt its caller's data, however
badly it scribbles.

### Edit the copy, return it — the revised-value pattern

Put the two facts together and the copy becomes a feature. The parameter
*is already* your working copy — edit it and return it:

```cpp
FrameMetadata with_frame_id(FrameMetadata metadata, int frame_id) {
    metadata.frame_id = frame_id;   // edit MY copy
    return metadata;                // hand the revised value to the caller
}

FrameMetadata original{7, "wrist-camera"};
FrameMetadata updated = with_frame_id(original, 8);
original.frame_id   // -> 7               the record of what happened, preserved
updated.frame_id    // -> 8
updated.source      // -> "wrist-camera"  carried along by the member-wise copy
```

You know this pattern from Python as the copying verbs — `sorted(lst)`
versus `lst.sort()`, or `dataclasses.replace(rec, frame_id=8)`. C++ needs
no explicit copy line, because the by-value parameter already did it.

### When the copy is too expensive

Honesty about cost: copying is real work proportional to the object.
Copying `FrameMetadata` is an `int` and a short string — nothing. Copying
a vector of a million doubles is a million doubles. That is why this
lesson's default is not the whole story: lesson 04 adds `const T&`
(read without copying) and `T&` (mutate the caller *on purpose*), and the
three together form the parameter convention you will use everywhere. The
rule of this lesson stands regardless: plain `T` means copy, always.

## Muscle memory

Type these until they come out without thinking:

```cpp
std::vector<int> v = u;      // COPY — C++'s default, Python's opt-in
FrameMetadata b = a;         // member-wise copy: int, string text, everything
void f(FrameMetadata m)      // by value: m is f's private copy
m.frame_id = 9;              // edits the copy — caller unaffected (trap AND feature)
FrameMetadata with_x(FrameMetadata copy, int x) {   // revised-value pattern
    copy.frame_id = x;
    return copy;
}
```

## The drills

One function in `starter.cpp`; `main` asserts on the original *and* the
returned value, so both halves of the pattern are checked.

### `with_frame_id(metadata, frame_id)`

Return `metadata` revised to carry the new `frame_id`; the caller's
original must come back unchanged.

```cpp
FrameMetadata original{7, "wrist-camera"};
FrameMetadata updated = with_frame_id(original, 8);
original.frame_id   // -> 7
updated.frame_id    // -> 8
updated.source      // -> "wrist-camera"
```

Two lines: assign into the by-value parameter, return it. The tests check
both sides of the copy boundary — `original.frame_id` must still be `7`
(you could not have reached it if you tried), and `updated.source` must
still read `"wrist-camera"`, proving the member-wise copy carried the
string while you replaced the int. The starter already returns
`metadata`; the drill is realizing the parameter is yours to edit.

**Where you'll see it:** interviews flag "do not modify the input" on
array and string problems constantly, and this is the C++ reflex for it —
work on a copy, return the copy. The `with_x` naming is the builder-style
API you will meet in config and options objects across real codebases. In
robotics it is provenance: a capture record from the wrist camera is
evidence, and every pipeline stage that relabels, retimestamps, or crops
derives a *new* record so logging and replay can still see what the sensor
actually said. Inference services do the same with per-request
configuration — copy the base config, tweak the copy, never let one
request's overrides leak into the next.

## How to practice

Write your attempt in `starter.cpp`, then run the tests against it:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/03_values_and_copies -q
```

Drop the `PRACTICE=1` to run them against the reference `solution.cpp`.
The untouched starter fails at `assert(updated.frame_id == 8)` — the
revised value never got its revision — which is worth seeing once before
you fix it.
