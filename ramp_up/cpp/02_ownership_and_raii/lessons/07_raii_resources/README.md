# 07 — RAII Resources

This lesson applies deterministic object lifetime to a non-memory resource: a
sensor device that must be released on every exit path.

## The problem this lesson solves

Explicit `open(); work(); close();` cleanup is fragile. An early return or thrown
exception can skip `close()`, leaving a device active and the next operation
unable to acquire it safely.

## The lesson

RAII—Resource Acquisition Is Initialization—binds a resource to an object's
lifetime. `ScopedDevice` acquires in its constructor and releases in its
destructor. Normal return destroys local objects at the closing brace; exception
unwinding also destroys fully constructed locals while leaving the scope.

The guard stores a borrowed reference to the external flag, so copying it would
create two guards claiming the same release duty. Deleted copy operations enforce
one guard for one acquisition.

## How interviewers test this

You may be asked to make cleanup exception-safe, explain stack unwinding, or
show why a copied resource guard and manual paired cleanup are unsafe.

## Muscle memory

```cpp
{
    ScopedDevice device(active); // acquire
    perform_read();               // may return or throw
}                                 // release either way
```

Put cleanup in the destructor, then let every exit path share it.

## The drills

Complete `ScopedDevice` so construction sets `active` true and destruction sets
it false. `read_with_device` must observe the acquired state as `42`; its existing
throw path proves the destructor also runs during exception unwinding.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/02_ownership_and_raii/lessons/07_raii_resources -q
```

Drop `PRACTICE=1` to check the reference solution.
