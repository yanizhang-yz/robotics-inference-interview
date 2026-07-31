# 02 — Headers and Translation Units

## Problem

Share a robotics joint-limit interface between independently compiled source
files, then link those pieces into one executable.

## Mental model

A **declaration** tells the compiler a name, its type, and how it may be used.
A **definition** provides the thing itself: storage for a variable, the body
of a function, or the full layout of a class or struct. Headers expose the
declarations and definitions that every translation unit needs to see, such as
class definitions, inline functions, constants, and templates.
`joint_limits.hpp` defines `JointLimits` and declares `clamp_to_limits`;
`joint_limits.cpp` contains the one non-inline definition of
`clamp_to_limits`.

`#include` is textual inclusion: before compilation, the preprocessor replaces
the include line with the header's contents. `#pragma once` prevents that
header from being included more than once in a single translation unit.

Each `.cpp` file is compiled independently after its includes are expanded.
That resulting source is a **translation unit**. The compiler can check a call
to `clamp_to_limits` from its declaration, while the linker later combines the
translation units and connects that call to the definition in
`joint_limits.cpp`.

## Application

A joint controller needs a common description of physical travel limits.
`JointLimits` lets command-validation code ask whether a requested command is
safe while a shared clamp function supplies the bounded command that reaches
the actuator layer. Both pieces use the same declaration without copying it.

## Prediction

If you change only `solution.cpp`, which file must be recompiled before
linking? Predict first: only `solution.cpp` needs recompilation; the unchanged
`joint_limits.cpp` object can be linked again. (Changing the header would make
both dependent translation units candidates for recompilation.)

## Guided implementation

Implement `command_is_safe(double command, const JointLimits& limits)` in
`starter.cpp`. It should return whether `command` lies in the inclusive range
from `limits.lower` through `limits.upper`. Do not alter the shared header or
the clamp definition.

## Verification

Run the reference with:

```bash
.venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/02_headers_and_translation_units -q
```

Run the learner starter with:

```bash
PRACTICE=1 .venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/02_headers_and_translation_units -q
```

The shared runner compiles `solution.cpp` or `starter.cpp` together with
`joint_limits.cpp`, using C++20 and `-Wall -Wextra -Werror=return-type`. The untouched
starter is expected to fail its first safety assertion.

## Explain it

- A header shares declarations and the definitions that must be visible in
  each translation unit, including class definitions, inline functions,
  constants, and templates.
- This lesson keeps the non-inline `clamp_to_limits` definition in exactly one
  `.cpp` file, satisfying the One Definition Rule (ODR).
- Textual inclusion makes a header's declarations and needed definitions
  visible in each translation unit.
- The linker resolves references between independently compiled translation
  units to form the executable.

## Next connection

As a robotics codebase grows, headers can describe reusable messages, sensor
interfaces, and control contracts while source files keep their implementations
separate. This boundary lets one executable link the particular components a
robot needs.
