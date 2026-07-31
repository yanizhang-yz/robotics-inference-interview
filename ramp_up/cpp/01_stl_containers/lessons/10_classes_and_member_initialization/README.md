# 10 — Classes and Member Initialization

## Problem

## Application

An image sensor gives a pipeline a frame width, height, and one initial pixel
value. Build a `CameraFrame` object that keeps those facts together and
constructs a `width * height` pixel buffer filled with that value. Downstream
code may inspect the frame, but it must not be able to replace its dimensions
or pixel storage directly.

## Mental model

A class names a new kind of value. Its `private` section holds implementation
details; only the class's own member functions may access those members. Its
`public` section is the interface that callers can use. Here, the constructor
establishes a valid frame and the four `const` accessors inspect it. A `const`
member function promises not to modify the object, so it can be called on the
`const CameraFrame frame` in `main`.

The constructor's member-initializer list is the part after `:`:

```cpp
CameraFrame(std::size_t width, std::size_t height, std::uint8_t fill)
    : width_(width),
      height_(height),
      pixels_(width * height, fill) {}
```

It constructs each member directly. `pixels_(width * height, fill)` invokes
the `std::vector` constructor that creates that many pixels, each initialized
to `fill`; it is not an assignment after an empty vector has been made.

**Actual initialization order is declaration order, not the written order of
the member-initializer list.** In this class, `width_` is declared first,
`height_` second, and `pixels_` third, so that is the order in which they are
initialized. Reordering the list's lines does not change it. Keep the list in
declaration order so readers and compiler diagnostics agree, and never rely on
one member being initialized earlier just because its initializer appears
earlier in the list.

`std::vector` owns its dynamic pixel storage and releases it automatically
when the `CameraFrame` is destroyed. This class therefore needs no destructor,
copy constructor, or copy-assignment operator: it is an early example of the
Rule of Zero.

## Prediction

Before running the program, answer these questions:

1. For `CameraFrame frame(4, 3, 7)`, how many elements should `pixels_`
   contain, and what should the checksum be?
2. Why can `main` call `width()` on a `const CameraFrame`?
3. Can code outside `CameraFrame` write `frame.width_`? Which part of the
   class makes the answer true?
4. If the initializer list wrote `pixels_` before `width_`, which member would
   still be initialized first, and why?
5. Which object owns the pixel allocation, and why does that mean this class
   does not manually `delete` anything?

## Guided implementation

In `starter.cpp`, repair only the constructor's member-initializer list. Use
the three constructor arguments to initialize `width_`, `height_`, and
`pixels_`. Keep the member declarations and their order unchanged. The
accessors and checksum loop already expose the behavior that verifies your
frame construction.

## Verification

Run the reference program with:

```bash
.venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/10_classes_and_member_initialization -q
```

Run the learner starter with:

```bash
PRACTICE=1 .venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/10_classes_and_member_initialization -q
```

The shared runner compiles with C++20 and `-Wall -Wextra -pedantic`. The
untouched starter is expected to fail at `frame.width() == 4`.

## Explain it

- `CameraFrame` bundles related state behind a small public interface.
- The constructor uses a member-initializer list to construct the stored
  dimensions and pixel buffer in their valid state.
- Member declarations control real initialization order; initializer-list
  spelling does not override that order.
- `const` accessors let callers observe a frame without granting mutation.
- `std::vector` manages the buffer lifetime, allowing `CameraFrame` to follow
  the Rule of Zero.

## Next connection

The earlier container and algorithm lessons used bare vectors of samples. A
class gives a pipeline a way to state what those samples mean and which values
must agree, while still relying on the same standard-library ownership model.
