# 04 — `unique_ptr`

This lesson packages exclusive heap ownership into a value whose destructor
performs cleanup automatically.

## The problem this lesson solves

Factories often create objects whose lifetime must outlast the factory call.
Returning a raw `new` pointer leaves unclear who must `delete` it and makes leaks
or double deletion easy.

## The lesson

`std::unique_ptr<T>` owns one `T`. When the pointer leaves scope, its destructor
destroys the object automatically. The ownership is exclusive, so copying is
disabled: two independently destructing values must not both believe they own
the same object.

`std::make_unique<Frame>(...)` constructs the `Frame` and returns its owner in
one expression. A factory can return that owner by value. The live-object counter
in this drill makes the destructor boundary visible: zero before the scope, one
inside it, and zero again afterward.

## How interviewers test this

Expect to write a factory returning `std::unique_ptr`, explain why it is
move-only, and identify the exact scope where its pointee is destroyed.

## Muscle memory

```cpp
auto frame = std::make_unique<Frame>(width, height, fill);
frame->pixels.size(); // access the owned object
```

Use `make_unique`; avoid owning raw `new`/`delete` pairs.

## The drills

Implement `make_frame(width, height, fill)` so it returns one non-null owner.
The frame constructor already builds `width * height` pixels. The assertions
check dimensions, pixel count, checksum, and automatic cleanup.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/02_ownership_and_raii/lessons/04_unique_ptr -q
```

Drop `PRACTICE=1` to check the reference solution.
