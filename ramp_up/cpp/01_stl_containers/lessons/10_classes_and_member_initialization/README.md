# 10 — Classes and Member Initialization

## Card

A class names a new kind of value: `private` members hold the state, and
the `public` interface is the only way callers touch it.

```cpp
CameraFrame(std::size_t width, std::size_t height, std::uint8_t fill)
    : width_(width), height_(height), pixels_(width * height, fill) {}
```

The list after `:` constructs each member directly — no empty-then-assign.
**Members initialize in declaration order, not the order written in the
list.** A `const` member function promises not to modify the object, so
it can be called on a `const CameraFrame`.

## Predict

The initializer list is rewritten to name `pixels_` first, then `width_`
and `height_`. Which member is initialized first?

- A) `pixels_`, because its initializer now appears first in the list
- B) `width_`, because members initialize in declaration order
- C) Whichever the compiler picks — the order is unspecified

<!-- predict
answer: B
why-A: The list's spelling never controls order; reordering its lines changes nothing about what runs first.
why-B: Right — construction always follows the member declarations: `width_`, then `height_`, then `pixels_`.
why-C: The order is fully specified — always the declaration order of the members, on every compiler.
-->

## Drill

In `starter.cpp`, repair only the `CameraFrame` constructor's
member-initializer list so the three arguments initialize `width_`,
`height_`, and `pixels_` — the pixel buffer gets `width * height`
elements filled with `fill`. Keep the member declarations and their order
unchanged.

Manual check: `PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/10_classes_and_member_initialization -q`

## Takeaway

- The member-initializer list constructs each member directly into its
  valid state.
- Member declarations control real initialization order; the list's
  spelling does not.
- `std::vector` owns the buffer lifetime, so `CameraFrame` follows the
  Rule of Zero — no destructor or copy functions needed.

## Deep dive

An image sensor hands a pipeline a frame width, height, and one initial
pixel value; the class's job is to guarantee those facts describe the
same image from the moment a frame exists. The constructor is where that
guarantee is made: the member-initializer list after `:` constructs each
member directly, and `pixels_(width * height, fill)` invokes the
`std::vector` constructor that creates that many pixels already set to
`fill` — not an assignment into a vector that was first made empty.
Because `width_`, `height_`, and `pixels_` are `private`, downstream code
cannot replace the dimensions or pixel storage independently; it inspects
the frame through `const` accessors, which promise not to modify the
object and therefore work on the `const CameraFrame` in `main`.

The subtle rule is ordering: members are initialized in declaration order
— `width_`, then `height_`, then `pixels_` — no matter how the
initializer list is spelled. Keep the list in declaration order so
readers and compiler diagnostics agree, and never rely on one member
being ready just because its initializer is written earlier in the list.

Ownership completes the picture. `std::vector` owns the dynamic pixel
storage and releases it when the frame is destroyed, so the class needs
no destructor, copy constructor, or copy-assignment operator — an early
example of the Rule of Zero. Earlier lessons passed bare vectors of
samples; a class states what those values mean and which of them must
agree.
