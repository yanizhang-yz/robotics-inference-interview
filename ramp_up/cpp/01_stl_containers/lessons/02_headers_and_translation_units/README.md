# 02 — Headers and Translation Units

## Card

A **declaration** names something for the compiler; a **definition** is
the thing itself. Headers hold what every translation unit must see;
each non-inline definition lives in exactly one `.cpp` file.

```cpp
// joint_limits.hpp — pasted textually into every file that includes it
struct JointLimits { double lower; double upper; };
double clamp_to_limits(double command, const JointLimits& limits);
```

`#include` is textual pasting (`#pragma once` blocks double pastes). Each
expanded `.cpp` compiles independently as a **translation unit**; the
linker then joins them, wiring every call to the single definition in
`joint_limits.cpp`.

## Predict

You edit only `solution.cpp`, leaving the header and `joint_limits.cpp`
untouched. What must be recompiled before the next link?

- A) Only `solution.cpp`
- B) Both `solution.cpp` and `joint_limits.cpp`
- C) Nothing — the linker picks up source edits by itself

<!-- predict
answer: A
why-A: Right — translation units compile independently, so the unchanged `joint_limits.cpp` object is simply linked again.
why-B: `joint_limits.cpp` never saw the edit; only a change to the shared header would make both translation units candidates.
why-C: The linker only joins object files — turning edited source into a fresh object file is the compiler's job.
-->

## Drill

In `starter.cpp`, implement `command_is_safe` so it reports whether the
requested command lies in the inclusive range `limits.lower` through
`limits.upper`. Leave the shared header and the clamp definition alone.

Manual check: `PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/02_headers_and_translation_units -q`

## Takeaway

- A header shares declarations plus the definitions every translation
  unit must see: struct layouts, inline functions, constants, templates.
- The non-inline `clamp_to_limits` definition lives in exactly one
  `.cpp` file — the One Definition Rule.
- The linker resolves references between independently compiled
  translation units to form the executable.

## Deep dive

A declaration tells the compiler a name, its type, and how it may be
used; a definition provides the thing itself — storage for a variable,
the body of a function, the full layout of a struct. `joint_limits.hpp`
defines `JointLimits` and declares `clamp_to_limits`;
`joint_limits.cpp` holds the one non-inline definition of that function.

`#include` is textual inclusion: before compilation, the preprocessor
replaces the include line with the header's contents, and `#pragma once`
keeps a header from being pasted twice into the same file. Each `.cpp`
file, once its includes are expanded, is compiled independently — that
expanded source is a translation unit. The compiler can check a call to
`clamp_to_limits` from the declaration alone; the linker later combines
the translation units and connects that call to its single definition.

The payoff is a shared contract. A joint controller needs one common
description of physical travel limits, and `JointLimits` lets
command-validation code ask whether a request is safe while the shared
clamp function supplies the bounded command that reaches the actuator
layer — both against the same declaration, copied nowhere. As a robotics
codebase grows, headers describe reusable messages, sensor interfaces,
and control contracts while source files keep their implementations
separate, so one executable can link exactly the components a particular
robot needs.
