# 01 — Build, Compile, and Link

## Card

A C++ build is three stages, and each stage reports different mistakes:

1. **Preprocess** — expand `#include`s into one translation unit.
2. **Compile** — check syntax and types; emit an object file whose calls
   into other files are left as unresolved references.
3. **Link** — resolve those references across object files and libraries;
   emit the executable.

A missing semicolon dies in stage 2. A declared-but-undefined function
survives stage 2 and dies in stage 3.

## Predict

`double f(double);` is declared and called in `main`, but no definition
exists anywhere. What happens?

- A) It fails to compile — the compiler needs the body to check the call
- B) It compiles, then the linker fails to resolve `f`
- C) It builds, then crashes at runtime when `f` is called

<!-- predict
answer: B
why-A: The declaration alone lets the compiler type-check the call; bodies routinely live in other translation units.
why-B: Right — compilation trusts the declaration, and the linker is what must find exactly one definition.
why-C: The build never finishes: an unresolved reference stops the linker before any binary exists to run.
-->

## Drill

In `starter.cpp`, implement `clamp_joint_command` so the requested command
is constrained to the inclusive `[lower, upper]` range. Use `std::clamp`
from the existing `<algorithm>` include, not a chain of branches.

Manual check: `PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/01_build_compile_and_link -q`

## Takeaway

- Compiler errors are local to one source file; linker errors are about
  definitions missing across the whole program.
- An object file is machine code plus unresolved references.
- The linker resolves every reference to exactly one definition, or the
  executable is never produced.

## Deep dive

Building is a sequence, not one opaque action. Preprocessing textually
expands `#include` directives and conditional compilation, producing one
self-contained translation unit. Compilation checks that unit's syntax and
types and lowers it to an object file — machine code in which any function
defined elsewhere is recorded only as a named, unresolved reference.
Linking combines object files and libraries, resolves each reference to
exactly one definition, and writes the executable the operating system
runs.

That is why the two failure kinds feel so different. A syntax or type
mistake stops the compiler while a single file is being translated, and
the message points at a file and line. An undefined function passes
compilation wherever a declaration is visible and only fails at the end,
when the linker searches every object file and finds no definition —
which is also why linker messages name symbols rather than lines.

The drill itself is real robotics code in miniature: before a position,
velocity, or torque command reaches hardware, control code clamps it into
the joint's safe range — one small piece of the safety envelope around a
physical robot. Robot programs also rarely live in one file: drivers,
control logic, and the executable wire-up compile separately into object
files and only meet at link time, which is the next lesson.
