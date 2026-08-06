# 01 — Scope and Lifetime

This lesson turns braces into a lifetime map. You will trace when nested
sensor-session objects begin and end, including their exact destruction order.

## The problem this lesson solves

A local object's lifetime is not indefinite: it begins when execution reaches
its declaration and ends at the closing brace of its scope. Code that borrows
that object after the brace is wrong, so you must be able to locate the boundary.

## The lesson

Automatic local objects are destroyed when their scope exits. Within one scope,
destruction reverses construction order. An inner scope exits before execution
continues in its parent, so `inner-b` dies before `inner-a`, then an
`after-inner` event can occur while `outer` is still alive.

The provided `ScopeMarker` records its construction in the constructor and its
destruction in the destructor. `scope_trace()` therefore makes otherwise
invisible lifetime events observable without manual cleanup calls.

## How interviewers test this

**Prediction:** trace the exact nested construction and destruction order.

**Implementation:** implement `scope_trace()` with the required events.

**Follow-up:** explain early-return and exception cleanup.

**Evidence:** name each scope boundary and justify reverse destruction.

You may be shown nested declarations and asked to predict the exact destructor
sequence, or asked whether a pointer/reference remains valid after a brace.

## Muscle memory

```cpp
{
    ScopeMarker outer("outer", events);
    { ScopeMarker inner("inner", events); } // inner dies here
}                                            // outer dies here
```

Read every closing brace as a deterministic cleanup point.

## The drills

Implement `scope_trace()` with `outer`, then a nested scope containing
`inner-a` and `inner-b`. After that inner scope, append `after-inner`. The
complete returned trace must show reverse destruction and no event after its
referenced `events` vector is gone.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/02_ownership_and_raii/lessons/01_scope_and_lifetime -q
```

Drop `PRACTICE=1` to check the reference solution.

Continue to the [next lesson](../02_construction_and_destruction/).
