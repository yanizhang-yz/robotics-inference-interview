# 03 — Owners and Borrowers

This lesson gives names to responsibility: a frame vector owns its elements,
while a pointer or reference temporarily accesses those elements without owning.

## The problem this lesson solves

A raw address says where an object is, not who must destroy it. Treating every
pointer as an owner causes double deletion; forgetting the true owner's lifetime
creates dangling borrowers.

## The lesson

`std::vector<Frame>` owns its `Frame` elements: their lifetimes follow the
container's storage. `Frame&` and `Frame*` are borrowers. They do not delete the
frame. Use a reference when a result must exist and a pointer when absence is
valid; `find_frame` returns `nullptr` for that absent case.

Borrowing is only valid while the referent stays alive and at the same address.
Destroying the vector ends every element borrow. Operations such as growth can
reallocate vector storage and invalidate pointers/references even while the
vector itself remains alive. `checksum(const Frame&)` communicates a non-null,
read-only borrow and must not mutate the frame.

## How interviewers test this

**Prediction:** identify the owner, borrower, null result, and invalidation point.

**Implementation:** implement `find_frame` and read-only `checksum`.

**Follow-up:** explain how vector growth can end a borrow.

**Evidence:** name ownership, lifetime, aliasing, and nullability explicitly.

You may be asked to label owner, borrower, and nullable result, explain why no
`delete` belongs in `find_frame`, and identify a `push_back` invalidation risk.

## Muscle memory

```cpp
Frame& required = frames[0];           // non-null borrow
Frame* optional = find_frame(frames, id); // nullable borrow
const Frame& read_only = frames[0];    // non-null, read-only borrow
```

First ask “who owns it?”, then “what can invalidate this borrow?”

## The drills

Implement `find_frame(std::vector<Frame>&, int)` by returning the address of the
matching element or `nullptr`. Implement `checksum(const Frame&)` as a read-only
pixel sum. The assertions verify alias identity, absence, and unchanged storage.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/02_ownership_and_raii/lessons/03_owners_and_borrowers -q
```

Drop `PRACTICE=1` to check the reference solution.

Continue to the [next lesson](../04_unique_ptr/).
