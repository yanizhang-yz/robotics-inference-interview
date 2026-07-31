# 07 — Containers and Contiguous Storage

## Problem

Build one grayscale camera frame whose pixels all begin with the same value, then
compute its checksum. A 4-by-3 frame filled with `7` has 12 pixels and a
checksum of 84.

## Mental model

`std::vector<T>` is a **value that owns a contiguous sequence of `T` elements**.
For this lesson, `T` is `std::uint8_t`, so every element is one grayscale pixel.
The vector object itself is a small container value; its pixel elements live in
the storage the vector manages. `frame` is not an array of pixels embedded in
the variable named `frame`: it is an owner that knows where its element storage
is, how many elements it contains, and how much storage it can use before it
must grow.

`size()` is the number of elements, not the number of bytes. `data()` gives a
pointer to the first element's storage when the vector is non-empty. Because
the elements are contiguous, incrementing that pointer walks from one pixel to
the next, and `data() + size()` is the one-past-the-end position.

Copying a vector copies its elements into independently owned storage. Changing
the copied frame does not change the original frame. In contrast, a pointer,
reference, or iterator into a vector borrows access to its current elements.
If an operation such as `push_back` grows the vector past its capacity, it may
allocate new storage and invalidate those borrows. Read the element again from
the vector after such growth instead of using an old pointer, reference, or
iterator.

## Application

Inference code often represents a grayscale camera image as a flat frame: row
0 comes first, then row 1, and so on. The element for `(row, column)` is at
`row * width + column`. A contiguous `std::vector<std::uint8_t>` makes that
layout explicit and supplies one owned value that can be passed between a
camera adapter, preprocessing stage, and inference request.

`make_gray_frame` creates the initial image buffer. `checksum` reads the frame
through a `const` reference, so it neither copies the pixels nor changes them.

## Prediction

Before running the program, answer these questions:

1. For `make_gray_frame(4, 3, 7)`, what does `size()` return? Is that count in
   pixels, bytes, rows, or columns?
2. What does `frame.data()` point to for this non-empty frame? Why does
   `frame.data() + frame.size()` name the position immediately after the final
   pixel?
3. What is the type of one element in `std::vector<std::uint8_t>`? What range
   of grayscale values can that element represent?
4. If `auto copy = frame;` is followed by `copy[0] = 0`, what happens to
   `frame[0]`? Which vector owns the changed element storage?
5. If you save an iterator or reference to `frame[0]` and then `push_back`
   forces the vector to grow, may you use that saved iterator or reference?
   Why must the answer account for the vector's storage moving?

## Guided implementation

In `starter.cpp`, return a vector with `width * height` elements, all initialized
to `fill`. Then iterate over the supplied pixels, add each `std::uint8_t` value
to a `long long` total, and return that total.

## Verification

Run the reference with:

```bash
.venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/07_containers_and_storage -q
```

Run the learner starter with:

```bash
PRACTICE=1 .venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/07_containers_and_storage -q
```

The shared runner compiles with C++20 and `-Wall -Wextra -pedantic`. The
untouched starter is expected to fail at `frame.size() == 12`.

## Explain it

- `std::vector<std::uint8_t>` owns the frame's contiguous pixel elements.
- `size()` counts elements; `data()` exposes the first element's address for a
  non-empty frame.
- A vector copy gets its own elements and storage.
- Growth can move storage, invalidating pointers, references, and iterators
  into the old elements.

## Next connection

Use a vector when an inference stage needs an owned, resizable sequence of
same-typed values. Keep borrowed access short-lived when the owner might grow
or otherwise change its storage.
