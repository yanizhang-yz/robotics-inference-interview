# Module 01 — C++ values and standard containers

This module builds the everyday C++ habits needed to read and write ordinary
values and standard containers. Work through the lessons in order — each is a
short read followed by a hands-on drill — then combine everything in the
telemetry capstone.

## Prerequisites

General programming experience and a C++ compiler (`clang++` or `g++`). No
prior C++ needed.

## Lesson path

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

## Already know some C++?

If you can explain — without guessing — how a compile error differs from a
link error, what changes when a `std::vector<int>` is passed by value and
modified, when a parameter should be `const T&` versus `T&`, what makes a
returned raw pointer dangle, how a null pointer differs from an empty
`std::optional`, which container keeps bytes contiguous, what template
instantiation does, and in what order class members initialize — jump
straight to the capstone and return to any lesson the work exposes.

## Capstone — Telemetry Triage

The module ends with [capstone/](capstone/): a joint-telemetry triage CLI
written from a near-empty `starter.cpp` in three graded stages — parse the
stream, clamp and group per joint, then produce the triage report. It
integrates the ten lessons and introduces `std::string` parsing, `std::map`,
and `std::set` in real use.

## How to practice

Write your attempt in a lesson's `starter.cpp`, then run the tests against it:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/04_references_and_const -q
```

Drop the `PRACTICE=1` to run the tests against the reference `solution.cpp`.
The whole module: `uv run pytest ramp_up/cpp/01_stl_containers -v`.

Return to the [C++ curriculum](../README.md) or continue to
[Module 02 — Ownership and RAII](../02_ownership_and_raii/).
