# C++ Modules 02–06 Micro-Lessons Design

**Date:** 2026-08-05

**Repository:** `robotics-inference-interview`

**Status:** Approved design

## Purpose

Finish the concept-first C++ ramp-up by splitting modules 02–06 into focused
micro-lessons that follow the established module-01 pattern. The resulting
track should help an experienced programmer become productive in C++ quickly
by replacing assumptions from higher-level languages with precise mental
models for lifetime, ownership, copies and moves, runtime dispatch, memory
layout, and synchronization.

The lessons also prepare learners for C++ interviews. Each concept explains
what an interviewer is trying to discover, the misconception that a prompt is
likely to expose, and the reasoning a strong answer should demonstrate. The
existing downstream exercises under `questions/` remain unchanged.

## Scope

This work will:

- add the approved micro-lesson inventories to modules 02–06;
- give every new lesson a `README.md`, `starter.cpp`, `solution.cpp`, and
  `test_solution.py`;
- turn each module README into a concise lesson index, diagnostic fast path,
  capstone guide, and navigation page;
- preserve each module's current monolithic coded exercise as its capstone;
- make every lesson explicitly connect its concept to robotics or inference;
- add an interview lens to every lesson;
- copy all information required to begin a drill into `starter.cpp`;
- retrofit the ten existing module-01 lesson READMEs with a dedicated
  interview section and their starters with the same drill brief, while
  preserving learner implementations already present;
- add the self-contained drill brief to the module-01 capstone starter and to
  the five module-root capstone starters in modules 02–06;
- keep the C++20 compiler and pytest practice contract intact; and
- extend structural tests so the complete inventory and lesson contract stay
  consistent.

This work will not:

- add, remove, or rewrite exercises under `questions/`;
- add top-level C++ modules 07, 08, or 09—the repository has six modules;
- teach general programming fundamentals;
- turn the ramp-up into a complete ROS, CUDA, or robotics-framework course;
- require an interactive drill runner; or
- reveal reference implementations in starter comments.

## Curriculum Shape

The six top-level modules remain the stable map of the C++ track. Module 01
already contains ten micro-lessons. Modules 02–06 gain 35 lessons in total:

### Module 02 — Ownership and RAII

1. `01_scope_and_lifetime`
2. `02_construction_and_destruction`
3. `03_owners_and_borrowers`
4. `04_unique_ptr`
5. `05_ownership_transfer`
6. `06_span_views`
7. `07_raii_resources`

The progression begins with predictable scope exit, distinguishes ownership
from access, introduces exclusive heap ownership, and ends by generalizing
RAII beyond memory. Robotics examples include camera frames, borrowed tensor
views, device handles, log scopes, files, and locks.

### Module 03 — Move Semantics and Resource-Aware Values

1. `01_observing_copy_cost`
2. `02_temporaries_and_value_categories`
3. `03_move_construction`
4. `04_moved_from_state`
5. `05_copy_elision`
6. `06_noexcept_moves`
7. `07_rule_of_zero`
8. `08_rule_of_five`

The progression makes copy cost observable before introducing the vocabulary
that explains moves. It teaches normal application code through the Rule of
Zero before treating the Rule of Five as a low-level integration and interview
skill. Robotics examples transfer image buffers between pipeline stages and
swap double buffers without copying their pixels.

### Module 04 — Runtime Polymorphism

1. `01_interfaces_and_dynamic_dispatch`
2. `02_virtual_and_override`
3. `03_virtual_destructors`
4. `04_polymorphic_ownership`
5. `05_object_slicing`
6. `06_dispatch_cost`

The progression separates the interface idea from C++ dispatch syntax, then
adds safe destruction, ownership, slicing, and performance reasoning.
Robotics examples use interchangeable camera, lidar, and inference backends.

### Module 05 — Memory Layout and Cache Behavior

1. `01_storage_and_object_layout`
2. `02_alignment_and_padding`
3. `03_cache_lines_and_locality`
4. `04_traversal_and_contiguous_storage`
5. `05_aos_and_soa`
6. `06_allocation_and_reserve`

The progression moves from where bytes live to how layout and access order
affect throughput. Drills use deterministic correctness checks; timing output
may illustrate a concept but never decides pass or fail. Robotics and
inference examples include image traversal, tensor data, point clouds,
detection records, and allocation-sensitive loops.

### Module 06 — Threads, Atomics, and Queues

1. `01_thread_lifetime`
2. `02_lambda_captures`
3. `03_data_races_and_mutexes`
4. `04_raii_locks`
5. `05_atomics`
6. `06_condition_variables`
7. `07_bounded_queues`
8. `08_clean_shutdown`

The progression starts with thread ownership and captured state, then adds
mutual exclusion, atomic state, blocking protocols, backpressure, and clean
shutdown. Robotics examples connect camera capture, inference, and publishing
stages. Concurrency starters must fail quickly when incomplete rather than
hang, and tests must use timeouts and deterministic invariants.

## Lesson Teaching Contract

Each lesson introduces one primary concept and follows the content formula
already proven in module 01:

1. State what the learner will be able to reason about and implement.
2. Explain the concrete failure, cost, or design problem the concept solves.
3. Build the smallest accurate mental model of the C++ behavior.
4. Connect that model to a robotics or inference use case.
5. Explain how an interviewer tests the concept and what the prompt is trying
   to reveal.
6. Provide a short muscle-memory section for important syntax and decisions.
7. Describe focused drills that map one-to-one to functions or classes in
   `starter.cpp`.
8. Give the reference and learner practice commands and connect to the next
   lesson or capstone.

Every lesson README retains these required level-two sections in this order:

```text
## The problem this lesson solves
## The lesson
## How interviewers test this
## Muscle memory
## The drills
## How to practice
```

Additional subsections are allowed when they improve clarity. Explanations may
contrast C++ with higher-level language behavior, but the main path must not
assume one specific source language.

## Self-Contained `starter.cpp` Contract

Opening `starter.cpp` must be sufficient to understand and begin the exercise.
The learner must not need to open the README before coding. Each starter begins
with a concise comment block containing:

```text
DRILL BRIEF
Concept: the single C++ idea being practiced
Scenario: the robotics or inference situation
Implement: the named functions, methods, or type changes
Behavior: concrete input/output examples and important edge cases
Interview focus: what an interviewer is checking
Tests: what the assertions establish
Run: the PRACTICE=1 pytest command
Done when: the observable completion signal
```

The drill brief duplicates the actionable specification from the README on
purpose. The README is the teaching reference; the starter is the coding
workspace. Keeping the task in both places supports learners who study first
and learners who begin directly in the editor.

Starter comments may name relevant standard-library types or operations when
those are the subject of the lesson. They must not include the completed
algorithm, a copy-pasteable solution body, or hidden answer keys. Stubs must
compile and fail an assertion quickly whenever the lesson is meant to be a
runtime exercise.

Retrofitting module 01 adds this header without resetting or modifying a
learner's existing function bodies. In particular, currently modified
module-01 starter solutions remain learner-owned work. The same rule applies
to the module-01 capstone starter.

## Interview Lens

The interview section is part of teaching, not a separate question bank. It
should cover three layers where appropriate:

- **Prediction:** determine destruction order, aliasing, copy/move behavior,
  dispatch target, layout, or synchronization outcome from a small snippet.
- **Implementation:** write or repair the smallest function or class that
  demonstrates the concept.
- **Follow-up reasoning:** explain trade-offs, failure modes, complexity,
  performance consequences, or a safer alternative.

The section states what evidence the interviewer wants. Strong evidence
includes identifying owners and borrowers, naming the lifetime boundary,
tracing copies and moves, explaining which bytes are contiguous, or specifying
the invariant and wake-up condition protected by synchronization. It avoids
generic lists of trivia questions that are disconnected from the drill.

## Files and Navigation

Every new lesson directory contains:

```text
README.md
starter.cpp
solution.cpp
test_solution.py
```

Each module README becomes a compact entry point with:

- prerequisites;
- an ordered lesson table with concept and application columns;
- a diagnostic fast path for experienced C++ programmers;
- a capstone description;
- reference and learner test commands; and
- links to the previous and next modules.

The existing module-root coded exercises in modules 02–06 remain at their
current paths and serve as capstones. Their `starter.cpp` files also gain a
self-contained capstone drill brief. Their behavior and test commands remain
compatible.

## Testing Contract

All lesson programs use C++20 and the shared compiler flags:

```text
-std=c++20 -Wall -Wextra -Werror=return-type
```

Threading lessons add `-pthread`. Each program prints `ALL TESTS PASSED` as its
final non-empty output line on success. Each `test_solution.py` delegates to
the shared `run_cpp_lesson` fixture and supplies only lesson-specific sources,
flags, or timeouts when necessary.

The automated suite will verify:

- the exact approved lesson inventory for all six modules;
- the presence of all four contract files;
- the required README sections and their order;
- the self-contained drill-brief fields in every starter;
- compilation and behavior of every reference solution;
- module capstone compatibility;
- practice selection through `PRACTICE=1`;
- `-pthread` coverage for concurrency lessons; and
- fast, bounded behavior for incomplete concurrency starters.

Memory-layout tests assert results and structural facts, not machine-dependent
timing ratios. Concurrency tests assert invariants and shutdown behavior, not a
particular thread schedule.

## Completion Criteria

The expansion is complete when:

1. Modules 02–06 contain all 35 approved lesson directories.
2. Every new lesson satisfies the teaching, starter, and testing contracts.
3. All ten module-01 lesson READMEs contain the dedicated interview section.
4. All ten module-01 lesson starters contain the drill brief without losing
   learner work.
5. All six module capstone starters contain self-contained drill briefs.
6. Module READMEs provide concise navigation rather than monolithic lessons.
7. The entire reference suite passes.
8. Structural tests enforce the complete curriculum contract.
9. No file under `questions/` has changed.

## Risks and Mitigations

### The expansion becomes too large to learn from

Keep one primary new concept per lesson and one small, realistic exercise.
Module indexes and diagnostic fast paths let experienced learners skip known
material without losing the dependency map.

### Starter comments become disguised solutions

Specify observable behavior, inputs, outputs, and edge cases while leaving the
algorithmic steps for the learner. Review starter and solution side by side to
remove any comment that makes the implementation mechanical.

### Duplicated drill text drifts

Treat the README drill section as the complete teaching version and the starter
brief as its concise coding contract. Structural tests enforce the starter
fields; implementation review checks that names, examples, and edge cases
agree across README, starter, solution, and tests.

### Concurrency lessons hang or fail nondeterministically

Use fail-fast starter behavior, bounded waits, explicit shutdown protocols,
and test timeouts. Do not require a data race to manifest for a test to pass.

### Existing learner work is overwritten

Limit the module-01 retrofit to comment headers around existing code. Inspect
the worktree before each edit and preserve any modified starter bodies.

## Alternatives Considered

### Fewer, larger lessons

This would require fewer files but would repeat the current failure mode:
learners would encounter several unfamiliar C++ mechanisms before they could
isolate which mental model was wrong.

### Interview-question-first lessons

This would support short-term memorization but would not build the lifetime,
ownership, layout, and synchronization reasoning needed to adapt when an
interviewer changes the prompt.

### Move drill instructions entirely out of the README

This would optimize editor-first practice at the cost of incomplete GitHub and
concept-tour documentation. Purposeful duplication gives both study modes a
complete experience, provided the contracts are kept synchronized.
