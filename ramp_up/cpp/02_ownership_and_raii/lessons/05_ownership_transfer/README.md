# 05 — Ownership Transfer

This lesson traces one camera frame as exclusive responsibility moves from
capture, through a relay, into a consuming inference stage.

## The problem this lesson solves

Exclusive ownership is useful only if it can change hands. Copying an owner
would create two cleanup responsibilities; passing a raw pointer would hide
which stage is responsible.

## The lesson

`std::move(owner)` permits a `std::unique_ptr` move. It does not move the
`Frame` itself: it transfers the owning pointer, preserving the pointee's
address. The source `unique_ptr` becomes null, making the responsibility change
visible and testable.

A by-value `std::unique_ptr<Frame>` parameter is a consuming interface. The
caller must transfer ownership to call it. Returning that parameter can relay
the same owner; letting it leave the final consumer's scope destroys the frame.

## How interviewers test this

You may be asked to trace which pointer is null after each move, distinguish
moving the owner from moving the pointee, and explain a by-value owner parameter.

## Muscle memory

```cpp
auto next = relay_frame(std::move(captured));
// captured == nullptr; next owns the same Frame address
consume_frame(std::move(next));
```

After a move, reassign or destroy the source; do not dereference it.

## The drills

Implement `relay_frame(std::unique_ptr<Frame>)` by returning its owner, and
`consume_frame(std::unique_ptr<Frame>)` by returning its checksum. Do not
allocate a replacement frame: address identity is part of the contract.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/02_ownership_and_raii/lessons/05_ownership_transfer -q
```

Drop `PRACTICE=1` to check the reference solution.
