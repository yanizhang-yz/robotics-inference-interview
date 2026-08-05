# 01 — Build, Compile, and Link

This lesson gets you from a `.cpp` file to a running program with one command,
and — more importantly — teaches you to read the two kinds of build failure
without panic: the compile error (points at a file and line) and the link
error (points at a missing symbol). After it, you will build and run every
drill in this track yourself, and you will write your first real function: the
bounds clamp that robotics code runs before any command reaches hardware.
Every output shown in a comment was run and verified.

## The problem this lesson solves

Coming from an interpreted language, the build step is the first wall: you
cannot "just run the file", and the first mistake you make greets you with a
half-screen of compiler output. Beginners read none of it and guess. The
skill is knowing that C++ has exactly two failure stages, that each produces a
recognizably different message, and that each tells you *where to look* — a
compile error means "this line of this file is wrong", a link error means "a
definition is missing somewhere in the whole program". Once you can tell them
apart, the wall of text collapses into one useful sentence.

## The lesson

### One command builds one program

In Python the gap between "file" and "running program" is one step:
`python file.py`. C++ splits it in two — *build*, then *run* — and the build
is where a whole class of mistakes gets caught before the program ever
starts. A C++ **compiler** turns source text into an executable file. On this
repo's toolchain that is `clang++` (or `g++` — same flags, same behavior
here):

```bash
clang++ -std=c++20 -Wall -Wextra -Werror=return-type starter.cpp -o program
./program
```

Reading the flags once, left to right: `-std=c++20` selects the 2020 edition
of the language (the one this track targets); `-Wall -Wextra` turn on the
useful **warnings** — messages about legal-but-suspicious code that cost
nothing to heed; `-Werror=return-type` upgrades one especially deadly warning
(a function that promises a return value but can exit without one) into a hard
error; `-o program` names the output file. Without `-o` you get a file
literally named `a.out`. There is no separate "run" tool: the output *is* a
program your operating system executes directly — that is what `./program`
does.

### The three stages inside that one command

The single command above actually runs a pipeline, and each stage can reject
your code differently:

1. **Preprocessing** — lines starting with `#`, like `#include <algorithm>`,
   are handled first. `#include` literally pastes in the named file, so the
   compiler sees one big self-contained unit of text (a **translation
   unit**).
2. **Compilation** — the compiler checks that translation unit's syntax and
   types, then turns it into an **object file**: machine code, plus a list of
   IOUs — names it *used* but that are defined elsewhere.
3. **Linking** — the **linker** gathers object files and libraries, pays off
   every IOU by finding exactly one definition for each name, and writes the
   executable.

You do not see the stages on success. You meet them the first time something
fails, because *which stage* failed is the first thing every error message
tells you — if you know how to look.

### Compile errors: a file and a line

Delete a semicolon and the **compiler** objects while reading that one file:

```cpp
int x = 1        // <- missing semicolon
return x;
```

```text
semi.cpp:2:14: error: expected ';' at end of declaration
    2 |     int x = 1
      |              ^
      |              ;
```

Read it like an address, left to right: *file* `semi.cpp`, *line* 2, *column*
14, then the category (`error:`) and the complaint. The caret points at the
exact spot, and here clang even prints the fix. Two habits worth forming on
day one: **always fix the first error first** (one real mistake often
cascades into dozens of follow-on errors — the fifteen errors below the first
one usually vanish when you fix it), and read the line number *before* the
prose.

### Link errors: a symbol, not a line

Now the other failure. This file *declares* a function — announces its name
and types, promising a definition exists — calls it, but never defines it:

```cpp
double f(double);                              // declaration: a promise
int main() { return static_cast<int>(f(1.0)); }
```

Compilation succeeds — the declaration gave the compiler everything it needed
to type-check the call. The **linker** then tries to pay the IOU and finds no
definition anywhere:

```text
Undefined symbols for architecture arm64:
  "f(double)", referenced from:
      _main in undef-654ecf.o
ld: symbol(s) not found for architecture arm64
```

Note what is *missing*: no file, no line, no caret. Link errors name a
**symbol** — the function or variable that has no definition — because the
linker works on compiled object files, after line numbers are gone. (`ld` is
the linker's own name; the exact wording varies by platform, but "undefined
symbol/reference" is the universal tell.) The diagnosis is always the same
question: *who was supposed to define this?* A typo'd name, a file you forgot
to compile in, a library you forgot to link. That is the whole trick: **file
and line → fix that code; symbol → find the missing definition.**

### The drill file, top to bottom

Open `starter.cpp` and read it as three parts:

```cpp
#include <algorithm>   // pastes in the standard algorithms (std::clamp lives here)
#include <cassert>     // assert
#include <iostream>    // std::cout printing

double clamp_joint_command(double command, double lower, double upper) { ... }

int main() {
    assert(clamp_joint_command(0.4, -1.5, 1.5) == 0.4);
    // ...
    std::cout << "ALL TESTS PASSED\n";
}
```

`main` is the program's fixed entry point — execution starts there.
**`assert(cond)`** is the simplest test tool in the language: if `cond` is
false it prints the failing expression with its file and line, and stops the
program on the spot; if true, it costs nothing and execution continues. So
the file is its own test: reach the final line and every assertion above it
held. That `ALL TESTS PASSED` line is this track's contract — the pytest
harness looks for it.

### `std::clamp`: bounds in one word

The drill itself needs one library tool. **`std::clamp(v, lo, hi)`** (from
`<algorithm>`) returns `v` pulled into the inclusive range `[lo, hi]` — it
returns the answer, it never modifies its arguments:

```cpp
std::clamp(0.4, -1.5, 1.5)    // -> 0.4    already in range: unchanged
std::clamp(2.0, -1.5, 1.5)    // -> 1.5    too high: pulled down to hi
std::clamp(-9.0, -1.5, 1.5)   // -> -1.5   too low: pulled up to lo
```

You could write the same thing as an `if`/`else if` chain, and it would work.
Prefer `clamp` anyway: it states the *intent* in one word, it cannot get a
boundary comparison backwards, and — the part that matters in a code review —
the argument order `(value, low, high)` is a convention every C++ reader
already knows. The one trap: *you* must pass `lo <= hi`; `clamp(x, 1.5, -1.5)`
is a bug the library does not catch for you.

## How interviewers test this

- **Prediction probe:** Given a build failure, classify it as a compiler failure or a linker failure from its file-and-line versus missing-symbol clues.
- **Implementation probe:** Repair `clamp_joint_command` so it constrains a requested joint command to its inclusive limits.
- **Follow-up probe:** Explain which build stage reports each kind of failure and what information it can report.

## Muscle memory

Type these until they come out without thinking:

```bash
clang++ -std=c++20 -Wall -Wextra -Werror=return-type file.cpp -o program && ./program
```

```cpp
std::clamp(value, lo, hi)   // bounds in one word — value first, then the range
assert(f(x) == expected);   // the poor man's test — file:line on failure
// compile error -> file:line:col, fix the FIRST one
// link error    -> undefined symbol, find the missing definition
```

## The drills

Work through `starter.cpp`. One function here — the point of this lesson is
the build loop around it as much as the code inside it.

### `clamp_joint_command(command, lower, upper)`

Return `command` constrained to the inclusive `[lower, upper]` range.

```cpp
clamp_joint_command(0.4, -1.5, 1.5)    // -> 0.4    in range already
clamp_joint_command(2.0, -1.5, 1.5)    // -> 1.5    the request exceeded the joint's limit
clamp_joint_command(-2.0, -1.5, 1.5)   // -> -1.5
```

This is `std::clamp` wearing a domain name. The starter already includes
`<algorithm>`; resist the `if` chain. While you are here, break the build on
purpose twice — delete a semicolon (compile error: file and line), then
misspell `clamp_joint_command` in one place (watch which stage catches it) —
and read both messages with the two-question habit from the lesson.

**Where you'll see it:** every robotics stack clamps commands into a joint's
safe envelope before they reach hardware — this exact function sits between
"the planner asked for 2.0 rad" and "the motor is rated for ±1.5 rad". The
same one-liner saturates pixel values in image processing, clips actions to
an environment's bounds in RL, and clips gradients in training loops. In
interviews it appears inside nearly every array problem as the "keep the
index in bounds" line — and writing `std::clamp` instead of a hand-rolled
`if` chain reads as fluency.

## How to practice

Write your attempt in `starter.cpp`, then run the tests against it:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/01_build_compile_and_link -q
```

Drop the `PRACTICE=1` to run them against the reference `solution.cpp`. Or
skip the harness entirely and use the one-command build loop from the lesson —
that habit is half of what this lesson is for.
