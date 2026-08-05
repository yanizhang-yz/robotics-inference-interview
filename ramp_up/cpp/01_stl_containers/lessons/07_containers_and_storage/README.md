# 07 — Containers and Contiguous Storage

## Card

`std::vector<T>` is a value that **owns a contiguous sequence** of elements.

```cpp
std::vector<std::uint8_t> frame(width * height, fill);  // owned pixels
```

`size()` counts elements, not bytes; `data()` points at the first element,
so `data() + size()` is one past the last pixel. Copying a vector copies
the elements into independently owned storage. But if `push_back` pushes
`size()` past `capacity()`, the vector reallocates — and every pointer,
reference, and iterator into the old elements is invalidated.

## Predict

You save a pointer to `frame[0]`, then a `push_back` pushes `size()` past
`capacity()`. Can you still use that pointer?

- A) Yes — the vector updates saved pointers to its new storage
- B) No — the vector reallocated, so the pointer aims at the old storage
- C) Only if you had saved a reference or iterator instead of a pointer

<!-- predict
answer: B
why-A: Nothing tracks raw pointers; the elements move to new storage and the pointer keeps the stale address.
why-B: Right — forced growth reallocates the element storage, invalidating every pointer into the old buffer.
why-C: References and iterators are invalidated by the same reallocation; re-read the element from the vector instead.
-->

## Drill

In `starter.cpp`, implement `make_gray_frame` to return a vector of
`width * height` pixels all initialized to `fill`, then implement
`checksum` to add every pixel into a `long long` total through the
read-only reference.

Manual check: `PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/07_containers_and_storage -q`

## Takeaway

- `std::vector<std::uint8_t>` owns the frame's contiguous pixel elements;
  `size()` counts elements and `data()` names the first element's address.
- A vector copy gets its own elements in independently owned storage.
- Growth can move storage, invalidating pointers, references, and
  iterators into the old elements.

## Deep dive

A `std::vector<std::uint8_t>` frame is an owner, not an array embedded in
the variable: the vector object itself is small and knows where its
element storage lives, how many pixels it holds, and how much capacity
remains before it must grow. Because the elements are contiguous,
incrementing a pointer from `data()` walks pixel to pixel, and
`data() + size()` names the one-past-the-end position. Inference code
leans on that layout: a grayscale image becomes a flat frame, row 0 first,
with the pixel for `(row, column)` at `row * width + column` — one owned
value passed between a camera adapter, a preprocessing stage, and an
inference request.

Ownership is what makes copies and borrows behave differently. Copying a
vector copies its elements into independently owned storage, so editing
the copy leaves the original frame alone. A pointer, reference, or
iterator merely borrows access to the current elements: if `push_back`
makes `size()` exceed `capacity()`, the vector reallocates and every such
borrow dangles. Insertion without reallocation has narrower invalidation
rules, so consult the operation's contract before retaining a borrow —
and after forced growth, read the element again from the vector rather
than trusting an old pointer. The habit to carry forward: reach for a
vector when a stage needs an owned, resizable sequence of same-typed
values, and keep borrowed access short-lived whenever the owner might
grow or otherwise change its storage.
