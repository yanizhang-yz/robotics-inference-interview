# Module 01 — C++ values and standard containers

This module builds the everyday C++ habits needed to read and write ordinary values and standard containers. Work through the lessons in order, then use the capstone to combine the ideas in one small, testable program.

## Prerequisites

General programming experience is enough. You do not need prior C++ experience.

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

## Diagnostic

Before beginning, answer these questions in your own words:

1. How does a compile error differ from a link error?
2. What changes when a `std::vector<int>` is passed by value and then modified?
3. When should a parameter be `const T&`, and when should it be `T&`?
4. What determines whether a raw pointer returned from a function is still valid?
5. How does a nullable pointer differ from an absent `std::optional` value?
6. Which container keeps frame bytes contiguously in memory?
7. What does the compiler do when a function template is instantiated for a concrete type?
8. In what order are members initialized from a class member-initializer list?

If you can explain every answer without guessing, take the fast path: attempt the root capstone now, then return to any lesson exposed by the work.

## Root capstone

The capstone preserves six contracts in `starter.cpp`. Implement them without changing their signatures. Its exercises apply strings, maps, and sets alongside vectors and standard algorithms.

| Function | Contract | Expected complexity |
| --- | --- | --- |
| `reverseWords` | Return whitespace-separated words in reverse order, with one space between output words. | O(n) time, O(n) space |
| `charFrequencies` | Count each character in an `std::unordered_map<char, int>`. | O(n) average time, O(u) space |
| `topKSmallest` | Sort a by-value input and return its smallest `k` values without changing the caller’s vector. | O(n log n) time, O(n) input copy space |
| `groupByLength` | Group words by length while preserving order inside each group and sorted length keys. | O(n) time plus map insertion costs, O(n) space |
| `sumOfUnique` | Add each distinct integer exactly once using a `long long` result. | O(n) average time, O(u) space |
| `appendDoubled` | Append doubled copies of the original elements without retaining invalidated vector iterators or references. | O(n) time, amortized O(n) growth space |

Run the reference capstone:

```bash
uv run pytest ramp_up/cpp/01_stl_containers -v
```

Practice against your implementation:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers -v
```

Or compile and run the starter directly:

```bash
clang++ -std=c++20 -Wall -Wextra -Werror=return-type -o /tmp/stl starter.cpp && /tmp/stl
```

Without `PRACTICE=1`, pytest checks the reference `solution.cpp`.

Return to the [C++ curriculum](../../README.md) or continue to [Module 02 — Ownership and RAII](../02_ownership_and_raii/).
