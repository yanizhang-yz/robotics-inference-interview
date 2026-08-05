# 05 — Pointer Borrows

## Card

A raw pointer, in its one narrow modern-C++ role, is **nullable,
non-owning access** to an object someone else owns.

```cpp
const Sensor* find_sensor(const std::vector<Sensor>& sensors,
                          const std::string& name);  // nullptr = not found
```

`const Sensor` lets callers inspect but never modify through the
pointer; `*` says the result may be `nullptr` and must be checked first.
`&sensor` borrows the address of an existing element, and
`camera->rate_hz` is shorthand for `(*camera).rate_hz`. Nothing is
created, freed, or owned.

## Predict

The loop finds a match and returns `&sensor`. What does the returned
pointer point to?

- A) A fresh copy of the matching `Sensor`
- B) The `Sensor` element stored inside the vector
- C) The vector itself, positioned at the match

<!-- predict
answer: B
why-A: `&sensor` takes the address of an object that already exists — no `Sensor` is constructed, which is the whole point of borrowing.
why-B: Right — the loop reference aliases the element in place, so the caller borrows the vector's own storage and owns nothing.
why-C: The type is `const Sensor*`: it designates one element, not the container that owns it.
-->

## Drill

In `starter.cpp`, implement `find_sensor`: scan `sensors` with a
`const Sensor&` loop, return `&sensor` when the name matches, and return
`nullptr` when the loop ends without a match.

Manual check: `PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/05_pointer_borrows -q`

## Takeaway

- `nullptr` is the explicit "no borrowed object" result; `&sensor`
  borrows the address of the matching element.
- `->` reads a member through a pointer already proven non-null.
- The pointer owns nothing: vector destruction or reallocation ends the
  borrow's validity.

## Deep dive

In this narrow role, a raw pointer is nullable, non-owning access to an
object someone else owns. `find_sensor` creates no `Sensor`, destroys
none, and transfers no ownership: it either borrows access to a matching
element already stored in the vector or returns `nullptr` to say "not
found." Both halves of `const Sensor*` carry information — `const`
promises callers may inspect but not modify through the pointer, and `*`
warns that the result can be null and must be checked before
`camera->rate_hz` (shorthand for `(*camera).rate_hz`) is evaluated.

The borrow has a validity boundary. The pointer stays good only while
the vector exists and is not reallocated: destroy the vector and the
pointer dangles; let a later operation such as `push_back` grow the
storage and the elements move, leaving pointers to the old elements
dangling too. Do not retain the borrow across such a change — look the
sensor up again after the registry is updated.

An inference service for a robot may keep its camera and joint-encoder
configuration in one sensor registry. A lookup for `"wrist-camera"`
yields a borrowed view of its update rate; a lookup for optional
hardware yields `nullptr`, and the caller chooses a fallback instead of
assuming presence. Prefer values, references, and smart pointers for
their own roles — reach for a raw pointer exactly when "may be absent,
owned elsewhere" is the message the API needs, and check it at the
boundary where existence becomes required.
