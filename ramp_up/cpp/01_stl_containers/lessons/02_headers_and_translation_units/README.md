# 02 — Headers and Translation Units

This lesson takes you from one `.cpp` file to a real multi-file program: a
shared header two source files agree on, and one build command that compiles
them together. After it you can split code across files, and read the two
link errors that only multi-file programs produce — *undefined symbol* and
*duplicate symbol* — knowing exactly which file to fix. The drill is
deliberately three files: `joint_limits.hpp` holds the shared contract,
`joint_limits.cpp` implements the clamp, and your `starter.cpp` implements a
safety check against the same header. Every output shown in a comment was
run and verified.

## The problem this lesson solves

In Python, sharing code between files is `import`, and it happens while the
program runs: the module executes, produces objects, you use them. C++ has
no runtime import. Every program beyond toy size is many `.cpp` files,
each compiled separately, joined by the linker at the end — and what the
files "share" is text pasted in before compilation even starts. Both naive
workarounds fail at link time with no file and no line: leave a function's
body out of the build and you get `undefined symbol`; paste the body into a
header so every file has it and you get `duplicate symbol`. This lesson
makes both errors boring.

## The lesson

### `#include` is paste, not import

Python's `import` runs a module once and hands you a live module object.
`#include` does something far dumber: during **preprocessing** (stage 1 of
lesson 01's pipeline) the line is *replaced by the file's text*, as if you
had typed it there. A **header** (`.hpp`) is simply a file written to be
pasted. The two spellings tell the preprocessor where to look:

```cpp
#include <algorithm>          // angle brackets: the standard library
#include "joint_limits.hpp"   // quotes: your own project, this folder first
```

### A `struct` is a record you define

The header's first job is a shared type. A **struct** is a record: a named
bundle of fields, each with a type. Its closest Python cousin is a
`@dataclass` — fields, field access with `.`, no behavior required:

```cpp
struct JointLimits {
    double lower;
    double upper;
};   // <- this semicolon is required, and forgetting it is a rite of passage

const JointLimits limits{-1.5, 1.5};   // brace init fills fields in order
limits.lower    // -> -1.5
limits.upper    // -> 1.5
```

### Declarations promise, definitions deliver

Lesson 01 met a **declaration** as "a promise a definition exists". Now the
precise split: a declaration gives a name and its types — everything the
compiler needs to *type-check a call*. A **definition** is the thing
itself — the function body. The header carries the struct definition plus
the function declaration; the body lives elsewhere:

```cpp
// joint_limits.hpp — the whole shared contract
double clamp_to_limits(double command, const JointLimits& limits);   // declaration
```

(One loan from lesson 04: `const JointLimits&` means "read-only access to
the caller's `JointLimits`, without copying it". Inside the function you
just write `limits.lower` as if you held the object itself.)

```cpp
// joint_limits.cpp — the ONE definition
#include "joint_limits.hpp"
#include <algorithm>

double clamp_to_limits(double command, const JointLimits& limits) {
    return std::clamp(command, limits.lower, limits.upper);
}
```

### A translation unit is one `.cpp`, fully pasted

After preprocessing, each `.cpp` plus everything it pasted is one
self-contained **translation unit**, and each translation unit compiles
*independently* into its own object file (lesson 01's stage 2). To build a
multi-file program you list every `.cpp` — the compiler processes each,
then the linker joins them:

```bash
clang++ -std=c++20 -Wall -Wextra -Werror=return-type starter.cpp joint_limits.cpp -o program
```

The independence is the point: because `joint_limits.cpp` never sees your
`starter.cpp`, editing one does not require recompiling the other — real
build systems cache each object file and recompile only what changed. Both
files paste the *same* header, so both type-check against the same struct
layout and the same declaration. That is what a header is *for*: one
contract, agreed on everywhere, defined nowhere twice.

### Forget a file and the linker names the gap

Hand the compiler only `starter.cpp` and watch:

```bash
clang++ -std=c++20 -Wall -Wextra -Werror=return-type starter.cpp -o program
```

```text
Undefined symbols for architecture arm64:
  "clamp_to_limits(double, JointLimits const&)", referenced from:
      _main in starter-d3be2f.o
ld: symbol(s) not found for architecture arm64
```

Compilation *succeeded* — the declaration was enough to type-check the
call. The link then failed, and lesson 01's rule ("symbol → find the
missing definition") gets its most common answer: the definition exists,
in a file you didn't put on the command line. The fix is the command line,
not the code.

### One definition, exactly

The opposite mistake: "just define the function *in* the header, then
every file has it." Paste means every translation unit now contains its
own full copy of the body. Two files include such a header, and the linker
finds the same symbol twice:

```text
duplicate symbol 'twice(double)' in:
    main-ec2516.o
    telemetry-631b09.o
ld: 1 duplicate symbols
```

This is the **One Definition Rule**: across the whole program, each
function has exactly one definition. Hence the discipline you just saw in
`joint_limits.hpp`: struct definitions and function *declarations* go in
the header (a struct definition may be pasted into many translation units —
each just needs to see it once); every function *body* goes in exactly one
`.cpp`. (The exceptions — `inline` functions, templates — come in later
lessons.)

### `#pragma once` blocks the double paste

Headers include other headers, so the same header can arrive twice in one
translation unit. Without protection, the second paste redefines the
struct — this time a *compile* error, with file and line:

```text
./joint_limits_bad.hpp:1:8: error: redefinition of 'JointLimits'
```

**`#pragma once`** as the first line of a header means "if this file was
already pasted into this translation unit, skip it". Put it at the top of
every header you ever write; `joint_limits.hpp` starts with it.

## Muscle memory

Type these until they come out without thinking:

```cpp
#pragma once                          // first line of every header
#include "joint_limits.hpp"           // quotes: your headers
#include <algorithm>                  // angle brackets: the standard library
struct JointLimits { double lower; double upper; };    // type definition: header
double clamp_to_limits(double, const JointLimits&);    // declaration: header
// function BODY: exactly one .cpp — the One Definition Rule
// undefined symbol -> a .cpp is missing from the command line
// duplicate symbol -> a body snuck into a header
```

```bash
clang++ -std=c++20 -Wall -Wextra -Werror=return-type starter.cpp joint_limits.cpp -o program
```

## The drills

One function in `starter.cpp` — the multi-file layout around it is the
other half of the drill. `main` asserts on your function *and* on
`clamp_to_limits`, so a successful run proves the link worked.

### `command_is_safe(command, limits)`

Return whether `command` lies inside the inclusive range `limits.lower`
through `limits.upper`.

```cpp
const JointLimits limits{-1.5, 1.5};
command_is_safe(0.5, limits)    // -> true
command_is_safe(2.0, limits)    // -> false
command_is_safe(1.5, limits)    // -> true   the boundary itself is safe: inclusive
```

The shape is one boolean expression:
`command >= limits.lower && command <= limits.upper`. The trap is the
Python habit of chaining:

```cpp
limits.lower <= command <= limits.upper   // compiles… and is WRONG
```

```text
warning: comparisons like 'X<=Y<=Z' don't have their mathematical meaning [-Wparentheses]
```

C++ evaluates left to right: `limits.lower <= command` is a `bool` (`true`,
which is `1`), and `1 <= limits.upper` is then almost always true — the
chained version verifiably returns `true` for `command = 2.0`. Heed that
warning; write the `&&`. And leave `joint_limits.hpp` and
`joint_limits.cpp` untouched — the test compiles both sources together,
which is the lesson.

**Where you'll see it:** the inclusive range check is the opening move of
every interval interview problem — "Merge Intervals", "Insert Interval",
meeting-rooms variants all begin with "does this value fall inside
[lo, hi]?" In robotics it is the *validation* half of the safety layer:
lesson 01 clamped a command into the envelope, this predicate instead
*rejects* out-of-envelope requests so the planner can be told no — both
run against the same shared `JointLimits` contract, which is exactly how
real stacks share message and limit structs through headers. In inference
services the same check gates configuration: batch sizes, timeouts, and
rate limits validated against a bounds struct before a worker accepts them.

## How to practice

Write your attempt in `starter.cpp`, then run the tests against it:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/02_headers_and_translation_units -q
```

Drop the `PRACTICE=1` to run them against the reference `solution.cpp`.
Either way the harness compiles `joint_limits.cpp` alongside your file —
the two-file build from the lesson. Worth doing once by hand: build with
and without `joint_limits.cpp` on the command line, and read the
undefined-symbol error knowing exactly what it means.
