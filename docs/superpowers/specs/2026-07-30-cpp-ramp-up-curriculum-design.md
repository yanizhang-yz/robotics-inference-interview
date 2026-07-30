# Self-Paced C++ Ramp-Up Curriculum Design

**Date:** 2026-07-30

**Repository:** `robotics-inference-interview`

**Status:** Approved design

## Purpose

Redesign `ramp_up/cpp` into a self-paced, concept-first C++ curriculum for
experienced programmers who are new to C++. The curriculum must make the
existing interview material and robotics projects approachable without
assuming that the learner comes from Java or any other specific language.

The current six C++ exercises remain valuable as integration capstones. Their
long lessons, however, introduce too many new ideas at once. The redesign keeps
the six-topic organization and existing capstones while adding small,
application-driven lessons that teach each prerequisite independently.

## Problem

The current track is technically sound and its six reference exercises pass,
but its pacing is too steep for a learner without prior C++ experience:

- each README is roughly 400–550 lines;
- a single lesson often combines syntax, language semantics, runtime behavior,
  performance, and systems design;
- Lesson 02 introduces ownership, destructors, RAII, `unique_ptr`, and
  ownership transfer together;
- Lesson 03 immediately adds value categories, move semantics, copy elision,
  `noexcept`, and the Rule of Five;
- later interview questions require C++20 vocabulary such as `std::span` and
  `std::optional` that is not practiced as a first-class concept;
- the current Java-to-C++ framing makes the public repository appear specific
  to one source language;
- learners see large classes before they have practiced tracing the lifetime
  and ownership of smaller objects.

The result is a gap between understanding individual definitions and following
those definitions when they interact inside a complete class or robotics
pipeline.

## Goals

1. Teach C++ directly to experienced programmers, independent of source
   language.
2. Let learners understand the purpose and mental model of a concept before
   implementing it.
3. Connect every concept to a concrete robotics or inference application.
4. Preserve the current six modules and their capstone exercises.
5. Split prerequisite material into focused micro-lessons with small programs.
6. Support a fast concept tour, a guided path, and a diagnostic fast path.
7. Standardize the ramp-up and downstream C++ interview material on C++20.
8. Provide clear, fast-failing automated feedback.
9. Make the repository and interactive tutoring follow the same teaching
   sequence.
10. Define content rather than a calendar so learners can choose their own
    pace.

## Non-Goals

- Teaching programming fundamentals such as loops and basic conditionals from
  first principles.
- Covering all of the C++ language or standard library.
- Requiring a fixed number of days or weeks.
- Turning the ramp-up into a complete robotics framework or ROS 2 course.
- Replacing the existing C++ interview questions.
- Teaching manual `new`/`delete` as an application-level default.
- Requiring advanced lock-free programming before the concurrency foundations
  are stable.

## Audience and Assumptions

The learner:

- has professional or substantial programming experience;
- understands functions, classes, containers, exceptions, and concurrency at a
  language-independent level;
- may come from Python, Java, JavaScript, Rust, Go, C#, or another language;
- has little or no C++ experience;
- wants to read and write C++ used in robotics, inference, and systems
  interviews.

Primary explanations must not use a source language as their organizing frame.
Java-specific sections, `JAVA:` annotations, and "what replaces Java" tables
will be removed from the C++ main path. Comparisons may describe general
managed-language or reference-semantics expectations only when that contrast
clarifies C++ itself.

## Teaching Principles

### Concept before implementation

Each unit first explains why the concept exists and what behavior it creates.
The learner predicts small examples before editing code. Implementation then
tests the mental model rather than serving as the first exposure to it.

### One primary concept per micro-lesson

A lesson may reuse earlier vocabulary but introduces only one primary idea.
Concepts that are commonly discussed together, such as RAII and `unique_ptr`,
receive separate first exposures before being combined.

### Application at the point of learning

Every lesson uses a robotics or inference situation, such as:

- owning a camera frame;
- borrowing a tensor view;
- releasing a device or file handle at scope exit;
- transferring a frame between pipeline stages;
- representing an absent sensor sample;
- aligning timestamped streams;
- choosing an observation memory layout;
- applying backpressure between capture and inference threads;
- shutting down a worker queue safely.

The application must explain why the mechanism matters for correctness,
latency, memory use, determinism, or safety.

### Rule of Zero before Rule of Five

The curriculum teaches containers and smart pointers as the normal ownership
tools. Learners practice the Rule of Zero before hand-writing resource
management. The Rule of Five remains an important recognition and interview
skill, but it is presented as low-level integration work rather than the
default style for application classes.

### Prediction and explanation are first-class work

Passing a runtime assertion is not sufficient. A learner must be able to
describe:

- when an object is destroyed;
- who owns a resource;
- who only borrows it;
- whether an expression copies, moves, or aliases;
- what synchronization boundary protects shared state;
- why the mechanism matters to the example application.

## Repository Architecture

Keep the current six top-level C++ module directories. Add a `lessons/`
directory inside each module. Keep the current root-level `starter.cpp`,
`solution.cpp`, and `test_solution.py` as the module capstone contract.

```text
ramp_up/cpp/
├── README.md
├── CONCEPT_MAP.md
├── LEARNING_POINTS.md
├── 01_stl_containers/
│   ├── README.md
│   ├── lessons/
│   │   ├── 01_build_compile_and_link/
│   │   ├── 02_headers_and_translation_units/
│   │   ├── 03_values_and_copies/
│   │   ├── 04_references_and_const/
│   │   ├── 05_pointer_borrows/
│   │   ├── 06_optional_results/
│   │   ├── 07_containers_and_storage/
│   │   ├── 08_algorithms_and_iteration/
│   │   ├── 09_introductory_templates/
│   │   └── 10_classes_and_member_initialization/
│   ├── starter.cpp
│   ├── solution.cpp
│   └── test_solution.py
├── 02_ownership_and_raii/
│   ├── README.md
│   ├── lessons/
│   │   ├── 01_scope_and_lifetime/
│   │   ├── 02_construction_and_destruction/
│   │   ├── 03_owners_and_borrowers/
│   │   ├── 04_unique_ptr/
│   │   ├── 05_ownership_transfer/
│   │   ├── 06_span_views/
│   │   └── 07_raii_resources/
│   ├── starter.cpp
│   ├── solution.cpp
│   └── test_solution.py
├── 03_move_semantics_rule_of_five/
│   ├── README.md
│   ├── lessons/
│   │   ├── 01_observing_copy_cost/
│   │   ├── 02_temporaries_and_value_categories/
│   │   ├── 03_move_construction/
│   │   ├── 04_moved_from_state/
│   │   ├── 05_copy_elision/
│   │   ├── 06_noexcept_moves/
│   │   ├── 07_rule_of_zero/
│   │   └── 08_rule_of_five/
│   ├── starter.cpp
│   ├── solution.cpp
│   └── test_solution.py
├── 04_virtual_functions_and_vtables/
│   ├── README.md
│   ├── lessons/
│   │   ├── 01_interfaces_and_dynamic_dispatch/
│   │   ├── 02_virtual_and_override/
│   │   ├── 03_virtual_destructors/
│   │   ├── 04_polymorphic_ownership/
│   │   ├── 05_object_slicing/
│   │   └── 06_dispatch_cost/
│   ├── starter.cpp
│   ├── solution.cpp
│   └── test_solution.py
├── 05_memory_layout_and_cache/
│   ├── README.md
│   ├── lessons/
│   │   ├── 01_storage_and_object_layout/
│   │   ├── 02_alignment_and_padding/
│   │   ├── 03_cache_lines_and_locality/
│   │   ├── 04_traversal_and_contiguous_storage/
│   │   ├── 05_aos_and_soa/
│   │   └── 06_allocation_and_reserve/
│   ├── starter.cpp
│   ├── solution.cpp
│   └── test_solution.py
└── 06_threads_atomics_queues/
    ├── README.md
    ├── lessons/
    │   ├── 01_thread_lifetime/
    │   ├── 02_lambda_captures/
    │   ├── 03_data_races_and_mutexes/
    │   ├── 04_raii_locks/
    │   ├── 05_atomics/
    │   ├── 06_condition_variables/
    │   ├── 07_bounded_queues/
    │   └── 08_clean_shutdown/
    ├── starter.cpp
    ├── solution.cpp
    └── test_solution.py
```

`LEARNING_POINTS.md` will become a concise C++ reference, not a source-language
translation card. `CONCEPT_MAP.md` will be the concept-tour entry point and
show dependencies between the six modules.

## Learning Routes

### Concept tour

The learner reads `CONCEPT_MAP.md` and the mental-model section of each module.
Short examples and checkpoint questions provide a map of the whole track
without requiring implementation. This route supports learners who want an
overview before they begin coding.

### Guided path

The learner completes micro-lessons in order. Each lesson combines explanation,
prediction, a small implementation, tests, and an application connection. The
module capstone combines the completed concepts.

### Fast path

Each module README includes a diagnostic:

- predict two or three small code examples;
- answer ownership, lifetime, performance, or synchronization questions;
- run the capstone starter.

A learner who can explain and implement the diagnostic may skip directly to the
capstone. This route avoids forcing experienced C++ programmers through
material they already know.

The routes define dependencies, not schedules. A learner may complete the
material in days, weeks, or months.

## Module Content

### Module 01: Core language and STL

Teach the compile/link model, headers and translation units, values and copies,
references, `const`, pointers as non-owning optional access, `std::optional`,
contiguous containers, strings, maps, sets, algorithms, iteration,
introductory templates, and enough class and member-initializer syntax to
support Module 02.

Applications include copying robot configuration values, returning an optional
sensor lookup, storing image bytes contiguously, and processing timestamped
samples.

### Module 02: Ownership and RAII

Teach scope, object lifetime, construction and destruction order, owners and
borrowers, raw pointers and references as non-owning access, `std::unique_ptr`,
ownership transfer, `std::span` as a borrowed view, and RAII resource wrappers.

Applications include camera-frame ownership, tensor views, scoped logging,
device handles, files, and locks. The existing `Buffer`, `makeBuffer`,
`moveBuffer`, and `ScopedLogger` exercise remains the capstone.

### Module 03: Move semantics

Teach observable copy cost, temporaries, lvalues and rvalues, move construction,
`std::move`, moved-from state, copy elision, `noexcept`, the Rule of Zero, and
recognition or implementation of the Rule of Five.

Applications include handing a camera frame between pipeline stages and
swapping double buffers without copying pixels. The existing instrumented
`FrameBuffer` exercise remains the capstone.

### Module 04: Runtime polymorphism

Teach interface-style base classes, `virtual`, `override`, pure virtual
functions, virtual destructors, polymorphism through references and smart
pointers, object slicing, vtables, and dispatch cost.

Applications include interchangeable camera and lidar drivers and a sensor
polling interface. The existing `Sensor`, `Camera`, and `Lidar` exercise remains
the capstone.

### Module 05: Memory and cache

Teach object storage, `sizeof`, alignment, padding, cache lines, locality,
contiguous versus pointer-chasing storage, traversal order, AoS versus SoA,
allocation behavior, and `reserve()`.

Applications include tensor traversal, detection output layout, observation
buffers, and allocation-sensitive control loops. The existing measurement
exercise remains the capstone.

### Module 06: Concurrency and queues

Teach thread lifetime, joining, lambda captures, data races, mutexes, RAII
locks, atomics, condition variables, bounded queues, backpressure, and clean
shutdown semantics.

Applications include camera-to-inference handoff, worker loops, and queue
shutdown. The current concurrency exercise remains the capstone and will gain
an explicit shutdown connection to the downstream bounded-blocking-queue
interview question.

## Micro-Lesson Contract

Every micro-lesson contains:

```text
README.md
starter.cpp
solution.cpp
test_solution.py
```

The README follows this order:

1. **Problem:** the failure, cost, or design question that creates the need.
2. **Mental model:** the smallest precise explanation of the C++ behavior.
3. **Application:** a robotics or inference scenario where it matters.
4. **Prediction:** a small program or trace to reason about before running it.
5. **Guided implementation:** a short task with focused TODOs.
6. **Verification:** the command and the behavior the tests establish.
7. **Explain it:** three to five verbal checkpoint questions.
8. **Next connection:** the later lesson, capstone, or interview problem that
   uses the concept.

Content targets:

- one primary new concept;
- roughly 80–150 lines per README;
- roughly 30–80 lines per starter program;
- three to six focused TODOs;
- no unexplained syntax required to complete the task;
- no solution revealed in the starter comments;
- realistic names and applications rather than abstract `Foo`/`Bar` examples.

Targets are guardrails, not mechanical limits. A lesson may be shorter. If it
needs substantially more content, it should normally be split.

## Interactive Tutoring Contract

Interactive tutoring mirrors the repository:

1. Select the current micro-lesson or project construct.
2. Ask one prediction or ownership question at a time.
3. Evaluate the learner's mental model before asking for code.
4. Explain only the smallest missing concept.
5. Ask the learner to attempt the relevant TODOs.
6. Read compiler or test output together.
7. Return to a smaller example if the combined class is still unclear.
8. End with an application explanation and a pointer to the next dependency.

Complete reference solutions are not presented before the learner has built a
mental model and attempted the task. When an afternoon project exposes an
unfamiliar construct, tutoring maps it to the appropriate micro-lesson, teaches
the missing prerequisite, and then returns to the project.

## Toolchain and Practice Interface

The C++ ramp-up will use C++20 to match the downstream questions:

```text
-std=c++20 -Wall -Wextra -Werror=return-type
```

Concurrency lessons add `-pthread`. Documentation will include optional
AddressSanitizer and UndefinedBehaviorSanitizer commands for relevant lifetime
and memory exercises.

The existing practice convention remains:

```bash
# Reference implementation
uv run pytest ramp_up/cpp/<module-or-lesson> -v

# Learner implementation
PRACTICE=1 uv run pytest ramp_up/cpp/<module-or-lesson> -v
```

The repository's existing virtual environment remains a valid fallback when
`uv` cache access is unavailable.

## Testing and Feedback

Each `test_solution.py`:

- discovers `clang++` or `g++` and skips with a clear reason when unavailable;
- compiles `solution.cpp` by default and `starter.cpp` with `PRACTICE=1`;
- captures and displays compiler diagnostics;
- applies a runtime timeout;
- asserts observable behavior rather than implementation details where
  possible;
- ends successful programs with `ALL TESTS PASSED`;
- avoids stub behavior that can hang;
- uses bounded polling or explicit synchronization for concurrency tests.

The full test suite verifies:

- every reference micro-lesson;
- all six preserved capstones;
- public repository structure;
- C++20 compatibility;
- the documented practice-selection behavior.

CI will run the full reference suite. Practice starters are expected to compile
but fail their behavioral checks until the learner completes them, except when
a compile-time lesson intentionally teaches a compiler error. Such a lesson
must provide a separate working starter state and an explicit diagnostic
experiment so the normal practice suite remains understandable.

## Progress and Completion

Progress is based on demonstrated understanding, not elapsed time. A concept is
complete when the learner can:

1. predict the relevant behavior;
2. explain why the mechanism exists;
3. identify owners, borrowers, copies, moves, or synchronization boundaries;
4. implement the small exercise;
5. connect it to the robotics or inference application;
6. pass the automated test.

A module is complete when the learner passes its capstone and can explain the
capstone's main design decisions without relying on the reference solution.

## Compatibility and Migration

- Keep the six current module directory names.
- Keep each module's root-level capstone command valid.
- Preserve the capstone's behavioral coverage while updating compilation to
  C++20 and neutralizing source-language-specific comments.
- Replace each long module README with a short module index and diagnostic
  guide after its content has been migrated into micro-lessons.
- Rewrite the top-level C++ `LEARNING_POINTS.md` as a concise reference.
- Update the root repository README so the C++ track is described as being for
  experienced programmers new to C++.
- Do not modify the Python ramp-up as part of this redesign.
- Do not change the downstream interview-question contracts except where build
  consistency or navigation links require it.

## Risks and Mitigations

### Too many tiny lessons create navigation overhead

Keep the six top-level modules, provide one `CONCEPT_MAP.md`, and make every
module README a short ordered index with a diagnostic fast path.

### Application examples overshadow the language concept

Use the smallest realistic robotics example that demonstrates the mechanism.
The lesson remains about C++; the application explains why it matters.

### Learners memorize syntax without understanding ownership

Require predictions and verbal checkpoints before the capstone, and use tests
that observe destruction, copying, movement, or synchronization.

### Learners read everything without writing code

Support the concept tour intentionally, but make the guided path and capstones
the evidence of operational understanding.

### Concurrency exercises hang or behave nondeterministically

Use timeouts, fail-fast stubs, bounded waits, explicit shutdown contracts, and
deterministic behavioral assertions wherever possible.

### The migration breaks existing commands

Keep capstone files at their current paths and add structural tests for
documented commands before replacing the long READMEs.

## Alternatives Considered

### Flatten all concepts into top-level lesson directories

This creates a clean linear sequence but loses the six-topic map and produces a
noisy root directory. It also makes the existing capstones harder to discover.

### Add a separate beginner bridge and leave the six lessons unchanged

This is faster initially but duplicates explanations and forces learners to
choose between overlapping tracks. The original overloaded lessons would still
be a maintenance problem.

### Begin only from a robotics project

This is motivating but leaves hidden prerequisite gaps and makes interview
coverage unpredictable. The approved design instead embeds project
applications in the structured concept path and supports returning from project
code to the relevant micro-lesson.

### Approved approach

Keep the six top-level modules, split their prerequisite material into nested
micro-lessons, preserve the original exercises as capstones, and connect every
concept to a robotics or inference application.

## Success Criteria

The redesign is successful when:

- the C++ entry point is language-neutral;
- a learner can inspect the full concept map before coding;
- each micro-lesson introduces one primary concept;
- each concept has a meaningful robotics or inference application;
- ownership, lifetime, and move semantics are taught in separate first
  exposures before being combined;
- the six existing exercises remain runnable as module capstones;
- all reference micro-lessons and capstones pass under C++20;
- practice mode consistently selects learner starters;
- concurrency exercises fail safely instead of hanging;
- downstream C++ interview questions link back to their prerequisites;
- the learner can explain the relevant mental model, implement the exercise,
  and transfer it to project code.
