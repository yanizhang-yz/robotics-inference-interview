# Module 01 — C++ values and standard containers

This module builds the everyday C++ habits needed to read and write ordinary
values and standard containers — and it plays like a game, not a textbook.

## Play it

From the repo root:

```bash
./drill
```

Each rep is seconds of reading, then coding: an ~8-line concept card, one
prediction question, then a drill in real C++ that recompiles on every save
of `starter.cpp`. Wrong answers get escalating help — first the failing
assertion, then a deeper explanation, then (only if you ask) the solution
diff. Pass all ten lessons and the capstone unlocks: a whole program you
build from scratch, stage by stage.

Useful moves: `./drill 7` jumps to a lesson, `./drill status` shows your
streak and progress, `./drill review` re-serves a few passed drills to keep
them sharp, and `./drill doctor` checks your setup.

## Prerequisites

General programming experience and a C++ compiler (`clang++` or `g++`). No
prior C++ needed.

## Lesson map

| Lesson | Concept | Application |
| --- | --- | --- |
| [01 — Build, compile, and link](lessons/01_build_compile_and_link/) | The stages that turn source files into a program | Diagnose a compile error versus an unresolved symbol. |
| [02 — Headers and translation units](lessons/02_headers_and_translation_units/) | Declarations, definitions, and separate compilation | Share an interface without duplicating a definition. |
| [03 — Values and copies](lessons/03_values_and_copies/) | Value semantics and copy boundaries | Predict which object changes after a by-value call. |
| [04 — References and const](lessons/04_references_and_const/) | Read-only and mutable aliases | Choose a parameter type that communicates intended access. |
| [05 — Pointer borrows](lessons/05_pointer_borrows/) | Nullable, non-owning access and lifetime | Return an address only while its referent remains alive. |
| [06 — Optional results](lessons/06_optional_results/) | Owned values that may be absent | Model a lookup that has no result without a sentinel. |
| [07 — Containers and storage](lessons/07_containers_and_storage/) | `std::vector` ownership and contiguous frame storage | Keep grayscale frame bytes in one owned, contiguous vector. |
| [08 — Algorithms and iteration](lessons/08_algorithms_and_iteration/) | Range-based iteration and `std::sort` | Select frame IDs with a range-based loop, then sort the result with a standard algorithm. |
| [09 — Introductory templates](lessons/09_introductory_templates/) | Type-parameterized code and instantiation | Reuse one algorithm for several value types. |
| [10 — Classes and member initialization](lessons/10_classes_and_member_initialization/) | Object construction and member-initializer lists | Trace how a small value type is initialized. |

Every lesson README is card-first: the short card and prediction question on
top, a deep dive below for when you want the full story.

## Already know some C++?

If you can explain — without guessing — how a compile error differs from a
link error, what changes when a `std::vector<int>` is passed by value and
modified, when a parameter should be `const T&` versus `T&`, what makes a
returned raw pointer dangle, how a null pointer differs from an empty
`std::optional`, which container keeps bytes contiguous, what template
instantiation does, and in what order class members initialize — jump
straight to `./drill capstone` and return to any lesson the work exposes.

## Capstone — Telemetry Triage

The finale is [capstone/](capstone/): a joint-telemetry triage CLI you write
from a near-empty `starter.cpp`, with staged tests that unlock progressively
— parse the stream, clamp and group per joint, then produce the triage
report. It integrates the ten lessons and introduces `std::string` parsing,
`std::map`, and `std::set` in real use. `./drill capstone` runs it (the rail
gets you there automatically after lesson 10).

## Without the runner

Everything also works as plain files and pytest:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/04_references_and_const -q
```

Without `PRACTICE=1`, pytest checks the reference `solution.cpp` instead of
your starter. The whole module: `uv run pytest ramp_up/cpp/01_stl_containers -v`.

Return to the [C++ curriculum](../README.md) or continue to
[Module 02 — Ownership and RAII](../02_ownership_and_raii/).
