# 01 — Build, Compile, and Link

## Problem

Turn one small robotics control function into an executable, and learn which
part of the C++ build reports each kind of mistake.

## Mental model

Building is a sequence, not one opaque action. First, **preprocessing** expands
`#include` directives and conditional compilation into one translation unit.
Next, **compilation** checks C++ syntax and types, then converts that translation
unit into an object file containing machine code plus unresolved references.
Finally, **linking** combines object files and libraries, resolves those
references, and produces the executable binary.

A compiler error is local to source code: a missing semicolon or a misspelled
type prevents an object file from being made. A linker error happens later: a
function may be declared so compilation succeeds, but if no object file or
library defines it, the linker cannot finish the executable.

## Application

Before a position, velocity, or torque command reaches hardware, control code
enforces the joint's safe limits. Here the clamp turns a requested value such
as `2.0` into an allowed command such as `1.5`. That simple boundary is one
small part of the safety envelope around a physical robot.

## Prediction

Which fails first: a missing semicolon or a declared-but-undefined function?
Predict before reading on: the missing semicolon fails during compilation,
before the linker can run. The undefined function can compile when its
declaration is visible, then fails during linking when no definition is found.

## Guided implementation

Implement `clamp_joint_command(double command, double lower, double upper)`
in `starter.cpp`. Return `command` constrained to the inclusive `[lower, upper]`
range. The provided assertions cover an in-range command and commands beyond
both limits. Use the existing `<algorithm>` include rather than writing a
manual branch chain.

## Verification

Run the reference with `.venv/bin/python -m pytest . -q`. To run the learner
starter instead, use `PRACTICE=1 .venv/bin/python -m pytest . -q`. The shared
runner compiles with C++20 and `-Wall -Wextra -Werror=return-type` before running the
selected binary.

## Explain it

- After compilation, a source file has become an object file.
- The linker resolves referenced definitions across object files and libraries.
- The final binary owns the executable machine code that the operating system runs.

## Next connection

Robotics applications rarely live in one source file. A driver can handle a
camera or actuator, control logic can compute commands, and an executable can
wire them together. Splitting those responsibilities lets independent pieces
compile into object files, be tested or reused separately, and link into the
specific robot program that needs them.
