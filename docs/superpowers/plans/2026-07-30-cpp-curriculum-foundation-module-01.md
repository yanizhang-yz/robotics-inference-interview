# C++ Curriculum Foundation and Module 01 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Establish the reusable C++20 micro-lesson infrastructure and deliver the complete language-neutral Module 01 learning path while preserving its original STL exercise as the capstone.

**Architecture:** Add one shared Python compile/run helper and pytest fixture used by every C++ lesson, then build ten independently testable micro-lessons under the existing `01_stl_containers` module. Keep the module's current root-level starter, solution, and test as its capstone; replace the long README with a short module index after its concepts have moved into the micro-lessons.

**Tech Stack:** Python 3.11+, pytest, C++20, clang++ or g++, C++ standard library, GitHub Actions

## Global Constraints

- Target experienced programmers who are new to C++; do not organize explanations around Java or another source language.
- Use C++20 with `-std=c++20 -Wall -Wextra -Werror=return-type`.
- Keep the six current `ramp_up/cpp` module directory names.
- Keep every module's root-level capstone command valid.
- Keep reference mode as the default and select `starter.cpp` only when `PRACTICE=1`.
- Every micro-lesson contains `README.md`, `starter.cpp`, `solution.cpp`, and `test_solution.py`.
- Every micro-lesson introduces one primary concept and connects it to a robotics or inference application.
- Present the problem and mental model before the implementation exercise.
- A starter must compile and fail its behavioral check quickly; it must not hang.
- A successful executable must end with `ALL TESTS PASSED`.
- Do not modify the Python ramp-up in this plan.
- Do not change downstream interview-question behavior in this plan.

## Scope and Follow-Up Plans

This is the first independently testable delivery from the approved curriculum design. It creates the interfaces consumed by later plans:

1. Module 02: ownership, lifetime, RAII, `unique_ptr`, `std::span`
2. Module 03: copy cost, move semantics, Rule of Zero, Rule of Five
3. Module 04: runtime polymorphism and virtual destruction
4. Module 05: memory layout, locality, and allocation
5. Module 06: threads, synchronization, queues, and shutdown
6. Final integration: prerequisite links from C++ interview questions, full navigation audit, and curriculum-wide acceptance tests

Each later plan must use the runner and lesson contract defined here instead of creating another compile harness.

## File Structure

### Shared infrastructure

- Create `ramp_up/cpp/cpp_test_support.py` — compiler discovery, practice selection, C++20 compilation, execution, timeout, and diagnostics
- Create `ramp_up/cpp/conftest.py` — `run_cpp_lesson` pytest fixture
- Create `tests/test_cpp_test_support.py` — direct contract tests for the shared runner
- Create `tests/test_cpp_curriculum_structure.py` — entry-point, lesson-contract, neutral-framing, and Module 01 inventory checks
- Modify all six existing `ramp_up/cpp/*/test_solution.py` files — delegate to the shared runner and compile as C++20

### Curriculum entry points

- Create `ramp_up/cpp/README.md` — language-neutral routes, commands, and module index
- Create `ramp_up/cpp/CONCEPT_MAP.md` — self-paced dependency map and concept-tour questions
- Rewrite `ramp_up/cpp/LEARNING_POINTS.md` — concise language-neutral C++ reference
- Modify `README.md` — public, source-language-neutral ramp-up section

### Module 01 micro-lessons

- Create `ramp_up/cpp/01_stl_containers/lessons/01_build_compile_and_link/`
- Create `ramp_up/cpp/01_stl_containers/lessons/02_headers_and_translation_units/`
- Create `ramp_up/cpp/01_stl_containers/lessons/03_values_and_copies/`
- Create `ramp_up/cpp/01_stl_containers/lessons/04_references_and_const/`
- Create `ramp_up/cpp/01_stl_containers/lessons/05_pointer_borrows/`
- Create `ramp_up/cpp/01_stl_containers/lessons/06_optional_results/`
- Create `ramp_up/cpp/01_stl_containers/lessons/07_containers_and_storage/`
- Create `ramp_up/cpp/01_stl_containers/lessons/08_algorithms_and_iteration/`
- Create `ramp_up/cpp/01_stl_containers/lessons/09_introductory_templates/`
- Create `ramp_up/cpp/01_stl_containers/lessons/10_classes_and_member_initialization/`

### Module 01 capstone migration

- Rewrite `ramp_up/cpp/01_stl_containers/README.md` — short module index, diagnostic, capstone contract, and links
- Modify `ramp_up/cpp/01_stl_containers/starter.cpp` — language-neutral comments and C++20 command
- Modify `ramp_up/cpp/01_stl_containers/solution.cpp` — language-neutral comments and C++20 command

---

### Task 1: Shared C++20 Lesson Runner

**Files:**
- Create: `ramp_up/cpp/cpp_test_support.py`
- Create: `ramp_up/cpp/conftest.py`
- Create: `tests/test_cpp_test_support.py`
- Modify: `ramp_up/cpp/01_stl_containers/test_solution.py`
- Modify: `ramp_up/cpp/02_ownership_and_raii/test_solution.py`
- Modify: `ramp_up/cpp/03_move_semantics_rule_of_five/test_solution.py`
- Modify: `ramp_up/cpp/04_virtual_functions_and_vtables/test_solution.py`
- Modify: `ramp_up/cpp/05_memory_layout_and_cache/test_solution.py`
- Modify: `ramp_up/cpp/06_threads_atomics_queues/test_solution.py`

**Interfaces:**
- Produces: `find_cpp_compiler() -> str | None`
- Produces: `select_cpp_source(lesson_dir: Path, practice: bool) -> Path`
- Produces: `compile_and_run_cpp(lesson_dir: Path, binary_path: Path, *, practice: bool, compiler: str | None = None, extra_sources: tuple[Path, ...] = (), extra_flags: tuple[str, ...] = (), timeout: int = 30) -> str`
- Produces pytest fixture: `run_cpp_lesson(lesson_dir: Path, *, extra_sources: tuple[Path, ...] = (), extra_flags: tuple[str, ...] = (), timeout: int = 30) -> str`
- Consumes: `PRACTICE=1` from the process environment

- [ ] **Step 1: Write failing shared-runner tests**

Create `tests/test_cpp_test_support.py`:

```python
from __future__ import annotations

from pathlib import Path

import pytest

from ramp_up.cpp.cpp_test_support import (
    compile_and_run_cpp,
    find_cpp_compiler,
    select_cpp_source,
)


def test_select_cpp_source_switches_only_in_practice_mode(tmp_path: Path) -> None:
    (tmp_path / "solution.cpp").write_text("// reference\n")
    (tmp_path / "starter.cpp").write_text("// learner\n")

    assert select_cpp_source(tmp_path, practice=False).name == "solution.cpp"
    assert select_cpp_source(tmp_path, practice=True).name == "starter.cpp"


def test_select_cpp_source_reports_the_missing_file(tmp_path: Path) -> None:
    with pytest.raises(FileNotFoundError, match="solution.cpp"):
        select_cpp_source(tmp_path, practice=False)


@pytest.mark.skipif(find_cpp_compiler() is None, reason="requires a C++ compiler")
def test_compile_and_run_cpp_uses_the_selected_program(tmp_path: Path) -> None:
    (tmp_path / "solution.cpp").write_text(
        '#include <iostream>\n'
        'int main() { std::cout << "reference\\nALL TESTS PASSED\\n"; }\n'
    )
    (tmp_path / "starter.cpp").write_text(
        '#include <iostream>\n'
        'int main() { std::cout << "practice\\nALL TESTS PASSED\\n"; }\n'
    )

    reference_output = compile_and_run_cpp(
        tmp_path, tmp_path / "reference", practice=False
    )
    practice_output = compile_and_run_cpp(
        tmp_path, tmp_path / "practice", practice=True
    )

    assert "reference" in reference_output
    assert "practice" in practice_output


@pytest.mark.skipif(find_cpp_compiler() is None, reason="requires a C++ compiler")
def test_compile_and_run_cpp_surfaces_compiler_diagnostics(tmp_path: Path) -> None:
    (tmp_path / "solution.cpp").write_text("int main( { return 0; }\n")

    with pytest.raises(AssertionError, match=r"solution\.cpp failed to compile"):
        compile_and_run_cpp(tmp_path, tmp_path / "broken", practice=False)
```

- [ ] **Step 2: Run the runner tests and verify the import fails**

Run:

```bash
.venv/bin/python -m pytest tests/test_cpp_test_support.py -q
```

Expected: collection fails with `ModuleNotFoundError` for
`ramp_up.cpp.cpp_test_support`.

- [ ] **Step 3: Implement the shared runner**

Create `ramp_up/cpp/cpp_test_support.py`:

```python
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path


DEFAULT_FLAGS = (
    "-std=c++20",
    "-Wall",
    "-Wextra",
    "-Werror=return-type",
)


def find_cpp_compiler() -> str | None:
    return shutil.which("clang++") or shutil.which("g++")


def select_cpp_source(lesson_dir: Path, practice: bool) -> Path:
    source = lesson_dir / ("starter.cpp" if practice else "solution.cpp")
    if not source.exists():
        raise FileNotFoundError(f"missing C++ lesson source: {source}")
    return source


def compile_and_run_cpp(
    lesson_dir: Path,
    binary_path: Path,
    *,
    practice: bool,
    compiler: str | None = None,
    extra_sources: tuple[Path, ...] = (),
    extra_flags: tuple[str, ...] = (),
    timeout: int = 30,
) -> str:
    resolved_compiler = compiler or find_cpp_compiler()
    if resolved_compiler is None:
        raise RuntimeError("requires clang++ or g++ on PATH")

    source = select_cpp_source(lesson_dir, practice)
    command = [
        resolved_compiler,
        *DEFAULT_FLAGS,
        *extra_flags,
        *(str(path) for path in extra_sources),
        str(source),
        "-o",
        str(binary_path),
    ]
    compile_proc = subprocess.run(command, capture_output=True, text=True)
    if compile_proc.returncode != 0:
        raise AssertionError(
            f"{source.name} failed to compile:\n{compile_proc.stderr}"
        )

    try:
        run_proc = subprocess.run(
            [str(binary_path)],
            capture_output=True,
            text=True,
            timeout=timeout,
        )
    except subprocess.TimeoutExpired as exc:
        raise AssertionError(
            f"{source.name} exceeded the {timeout}-second runtime limit"
        ) from exc

    if run_proc.returncode != 0:
        raise AssertionError(
            f"{source.name} binary exited with {run_proc.returncode}\n"
            f"stdout:\n{run_proc.stdout}\n"
            f"stderr:\n{run_proc.stderr}"
        )

    non_empty = [line for line in run_proc.stdout.splitlines() if line.strip()]
    if not non_empty or non_empty[-1].strip() != "ALL TESTS PASSED":
        raise AssertionError(
            "expected final line 'ALL TESTS PASSED', got:\n"
            f"{run_proc.stdout}"
        )
    return run_proc.stdout
```

Create `ramp_up/cpp/conftest.py`:

```python
from __future__ import annotations

import os
from pathlib import Path
from typing import Callable

import pytest

from ramp_up.cpp.cpp_test_support import compile_and_run_cpp, find_cpp_compiler


@pytest.fixture
def run_cpp_lesson(
    tmp_path: Path,
) -> Callable[..., str]:
    compiler = find_cpp_compiler()
    if compiler is None:
        pytest.skip("requires clang++ or g++ on PATH")
    practice = os.environ.get("PRACTICE") == "1"

    def run(
        lesson_dir: Path,
        *,
        extra_sources: tuple[Path, ...] = (),
        extra_flags: tuple[str, ...] = (),
        timeout: int = 30,
    ) -> str:
        return compile_and_run_cpp(
            lesson_dir,
            tmp_path / "a.out",
            practice=practice,
            compiler=compiler,
            extra_sources=extra_sources,
            extra_flags=extra_flags,
            timeout=timeout,
        )

    return run
```

- [ ] **Step 4: Run the runner tests and verify they pass**

Run:

```bash
.venv/bin/python -m pytest tests/test_cpp_test_support.py -q
```

Expected: four tests pass, or compiler-dependent tests skip when no compiler is
installed.

- [ ] **Step 5: Replace the duplicated capstone harnesses**

Use this exact body for
`01_stl_containers/test_solution.py`:

```python
from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_stl_containers_capstone(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
```

Use this exact body for
`02_ownership_and_raii/test_solution.py`:

```python
from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_ownership_and_raii_capstone(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
```

Use this exact body for
`03_move_semantics_rule_of_five/test_solution.py`:

```python
from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_move_semantics_capstone(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
```

Use this exact body for
`04_virtual_functions_and_vtables/test_solution.py`:

```python
from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_virtual_functions_capstone(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
```

Use this exact body for
`05_memory_layout_and_cache/test_solution.py`:

```python
from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_memory_layout_capstone(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
```

Use this exact body for
`06_threads_atomics_queues/test_solution.py`:

```python
from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_threads_atomics_queues_capstone(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE, extra_flags=("-pthread",))
```

- [ ] **Step 6: Run every preserved capstone through the C++20 runner**

Run:

```bash
.venv/bin/python -m pytest ramp_up/cpp -q
```

Expected: the six current capstones pass under C++20.

- [ ] **Step 7: Commit the runner and C++20 migration**

```bash
git add ramp_up/cpp/cpp_test_support.py ramp_up/cpp/conftest.py \
  tests/test_cpp_test_support.py ramp_up/cpp/*/test_solution.py
git commit -m "test: share C++20 lesson runner"
```

### Task 2: Language-Neutral Curriculum Entry Points

**Files:**
- Create: `tests/test_cpp_curriculum_structure.py`
- Create: `ramp_up/cpp/README.md`
- Create: `ramp_up/cpp/CONCEPT_MAP.md`
- Rewrite: `ramp_up/cpp/LEARNING_POINTS.md`
- Modify: `README.md`

**Interfaces:**
- Produces: the public entry point `ramp_up/cpp/README.md`
- Produces: concept-tour entry point `ramp_up/cpp/CONCEPT_MAP.md`
- Produces: reference card `ramp_up/cpp/LEARNING_POINTS.md`
- Produces structural rule: every discovered micro-lesson has the four contract files and eight required README headings
- Consumes: the six existing module directory names

- [ ] **Step 1: Write failing entry-point and contract tests**

Create `tests/test_cpp_curriculum_structure.py`:

```python
from __future__ import annotations

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
CPP_ROOT = ROOT / "ramp_up" / "cpp"
ENTRY_DOCS = (
    ROOT / "README.md",
    CPP_ROOT / "README.md",
    CPP_ROOT / "CONCEPT_MAP.md",
    CPP_ROOT / "LEARNING_POINTS.md",
)
CONTRACT_FILES = (
    "README.md",
    "starter.cpp",
    "solution.cpp",
    "test_solution.py",
)
CONTRACT_HEADINGS = (
    "## Problem",
    "## Mental model",
    "## Application",
    "## Prediction",
    "## Guided implementation",
    "## Verification",
    "## Explain it",
    "## Next connection",
)


def test_cpp_curriculum_entry_docs_exist() -> None:
    missing = [str(path.relative_to(ROOT)) for path in ENTRY_DOCS if not path.exists()]
    assert missing == []


def test_cpp_entry_docs_are_source_language_neutral() -> None:
    forbidden = ("java", "arraylist", "hashmap", "treemap")
    violations = []
    for path in ENTRY_DOCS:
        text = path.read_text().lower()
        for phrase in forbidden:
            if phrase in text:
                violations.append(f"{path.relative_to(ROOT)}: {phrase}")
    assert violations == []


def test_discovered_micro_lessons_follow_the_contract() -> None:
    violations = []
    for lessons_dir in sorted(CPP_ROOT.glob("[0-9][0-9]_*/lessons")):
        for lesson_dir in sorted(path for path in lessons_dir.iterdir() if path.is_dir()):
            for filename in CONTRACT_FILES:
                if not (lesson_dir / filename).is_file():
                    violations.append(
                        f"{lesson_dir.relative_to(ROOT)}: missing {filename}"
                    )
            readme = lesson_dir / "README.md"
            if readme.exists():
                text = readme.read_text()
                for heading in CONTRACT_HEADINGS:
                    if heading not in text:
                        violations.append(
                            f"{lesson_dir.relative_to(ROOT)}: missing {heading}"
                        )
    assert violations == []
```

- [ ] **Step 2: Run the structural tests and verify the entry points are missing**

Run:

```bash
.venv/bin/python -m pytest tests/test_cpp_curriculum_structure.py -q
```

Expected: `test_cpp_curriculum_entry_docs_exist` fails for
`ramp_up/cpp/README.md` and `ramp_up/cpp/CONCEPT_MAP.md`; the neutral-framing
test also reports current source-language framing.

- [ ] **Step 3: Create the C++ curriculum README**

Create `ramp_up/cpp/README.md` with these exact sections and claims:

```markdown
# C++ for Robotics Inference

This self-paced track is for experienced programmers who are new to C++.
It teaches the language mechanisms that control ownership, lifetime, memory
layout, concurrency, and performance in robotics and inference systems.

## Choose a route

- Concept tour: read `CONCEPT_MAP.md` before implementing.
- Guided path: complete micro-lessons in order, then the module capstone.
- Fast path: answer a module diagnostic and attempt its capstone.

## Practice contract

Reference:
`uv run pytest ramp_up/cpp/<module-or-lesson> -v`

Learner attempt:
`PRACTICE=1 uv run pytest ramp_up/cpp/<module-or-lesson> -v`

## Modules

1. Core language and STL
2. Ownership and RAII
3. Move semantics and resource-aware values
4. Runtime polymorphism
5. Memory layout and cache behavior
6. Threads, atomics, and queues

## Completion

A concept is complete when you can predict its behavior, explain why it
exists, implement the exercise, pass the test, and connect it to the stated
robotics or inference application.
```

Add working relative links to `CONCEPT_MAP.md`, `LEARNING_POINTS.md`, and all
six module directories. Do not include duration estimates.

- [ ] **Step 4: Create the complete concept map**

Create `ramp_up/cpp/CONCEPT_MAP.md` with:

- a dependency chain from build/value semantics through ownership, movement,
  polymorphism, memory, and concurrency;
- a table mapping each module to its mental model, application, capstone, and
  downstream interview questions;
- a concept-tour section containing these questions:
  1. When is a local object destroyed?
  2. Who owns the resource and who only borrows it?
  3. Does this expression copy, move, or alias?
  4. Which bytes are contiguous?
  5. What synchronization boundary protects shared state?
  6. What event wakes a blocked producer or consumer?
- a route back to `README.md` and forward to Module 01.

- [ ] **Step 5: Rewrite the language-neutral reference card**

Replace `ramp_up/cpp/LEARNING_POINTS.md` with a concise C++20 reference using
these sections:

1. Program structure, compilation, linking, headers, and translation units
2. Values, references, pointers, and `const`
3. Containers, algorithms, iterators, `std::optional`, and `std::span`
4. Scope, lifetime, ownership, destructors, and RAII
5. Copies, moves, copy elision, Rule of Zero, and Rule of Five
6. Runtime polymorphism and virtual destruction
7. Object layout, allocation, cache locality, AoS, and SoA
8. Threads, mutexes, RAII locks, atomics, condition variables, and shutdown
9. Undefined behavior, compiler warnings, and sanitizer commands
10. Function-signature decision table for `T`, `const T&`, `T&`, `T*`,
    `std::span<T>`, and `std::unique_ptr<T>`

Each section must contain:

- a direct C++ rule;
- one short compilable C++20 example;
- one common mistake;
- one link to the module that teaches it.

Use `-std=c++20` in every command. Remove source-language comparison tables and
source-language code blocks.

- [ ] **Step 6: Rewrite the root ramp-up section**

In `README.md`:

- change `ramp_up/` to "language fluency tracks for experienced programmers";
- replace `## Language ramp-up (coming from Java?)` with
  `## Language ramp-up`;
- describe Python and C++ without "what it replaces" tables;
- link the C++ row to `ramp_up/cpp/README.md`;
- keep the existing question tables and question practice contract unchanged.

- [ ] **Step 7: Run entry-point and public-tree checks**

Run:

```bash
.venv/bin/python -m pytest \
  tests/test_cpp_curriculum_structure.py \
  tests/test_public_tree.py -q
```

Expected: all tests pass.

- [ ] **Step 8: Commit the curriculum entry points**

```bash
git add README.md ramp_up/cpp/README.md ramp_up/cpp/CONCEPT_MAP.md \
  ramp_up/cpp/LEARNING_POINTS.md tests/test_cpp_curriculum_structure.py
git commit -m "docs: add self-paced C++ curriculum entry points"
```

### Task 3: Micro-Lesson 01 — Build, Compile, and Link

**Files:**
- Create: `ramp_up/cpp/01_stl_containers/lessons/01_build_compile_and_link/README.md`
- Create: `ramp_up/cpp/01_stl_containers/lessons/01_build_compile_and_link/starter.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/01_build_compile_and_link/solution.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/01_build_compile_and_link/test_solution.py`

**Interfaces:**
- Produces: `double clamp_joint_command(double command, double lower, double upper)`
- Consumes: shared fixture `run_cpp_lesson`

- [ ] **Step 1: Write the lesson test first**

Create `test_solution.py`:

```python
from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_build_compile_and_link_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
```

- [ ] **Step 2: Run the test and verify the missing source failure**

Run:

```bash
.venv/bin/python -m pytest \
  ramp_up/cpp/01_stl_containers/lessons/01_build_compile_and_link -q
```

Expected: fail with `missing C++ lesson source` for `solution.cpp`.

- [ ] **Step 3: Add the reference and learner programs**

Create `solution.cpp`:

```cpp
#include <algorithm>
#include <cassert>
#include <iostream>

double clamp_joint_command(double command, double lower, double upper) {
    assert(lower <= upper);
    return std::clamp(command, lower, upper);
}

int main() {
    assert(clamp_joint_command(0.4, -1.5, 1.5) == 0.4);
    assert(clamp_joint_command(2.0, -1.5, 1.5) == 1.5);
    assert(clamp_joint_command(-2.0, -1.5, 1.5) == -1.5);
    std::cout << "ALL TESTS PASSED\n";
}
```

Create `starter.cpp` with the same includes and `main`, but use this exercise
body:

```cpp
double clamp_joint_command(double command, double lower, double upper) {
    // Exercise: return the command clamped to the inclusive [lower, upper] range.
    (void)lower;
    (void)upper;
    return command;
}
```

- [ ] **Step 4: Write the concept-first README**

Create the eight required headings. Explain:

- preprocessing, compilation, and linking as three distinct stages;
- a compiler error versus a linker error;
- why robotics projects split drivers, control logic, and executables;
- the prediction: whether a missing semicolon fails before a declared but
  undefined function;
- the guided task: implement `clamp_joint_command`;
- the application: enforcing a joint limit before a command reaches hardware;
- checkpoint answers expected: source becomes an object file, the linker
  resolves referenced definitions, and the final binary owns the executable
  machine code.

- [ ] **Step 5: Verify reference success and learner failure**

Run:

```bash
.venv/bin/python -m pytest \
  ramp_up/cpp/01_stl_containers/lessons/01_build_compile_and_link -q
PRACTICE=1 .venv/bin/python -m pytest \
  ramp_up/cpp/01_stl_containers/lessons/01_build_compile_and_link -q
```

Expected: reference passes; practice fails an assertion for an out-of-range
command.

- [ ] **Step 6: Commit the build lesson**

```bash
git add ramp_up/cpp/01_stl_containers/lessons/01_build_compile_and_link
git commit -m "feat: teach the C++ build pipeline"
```

### Task 4: Micro-Lesson 02 — Headers and Translation Units

**Files:**
- Create: `ramp_up/cpp/01_stl_containers/lessons/02_headers_and_translation_units/README.md`
- Create: `ramp_up/cpp/01_stl_containers/lessons/02_headers_and_translation_units/joint_limits.hpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/02_headers_and_translation_units/joint_limits.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/02_headers_and_translation_units/starter.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/02_headers_and_translation_units/solution.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/02_headers_and_translation_units/test_solution.py`

**Interfaces:**
- Produces: `struct JointLimits { double lower; double upper; };`
- Produces: `double clamp_to_limits(double command, const JointLimits& limits)`
- Lesson exercise produces: `bool command_is_safe(double command, const JointLimits& limits)`
- Consumes: `run_cpp_lesson(..., extra_sources=(HERE / "joint_limits.cpp",))`

- [ ] **Step 1: Write the multi-source test**

Create `test_solution.py`:

```python
from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_headers_and_translation_units_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE, extra_sources=(HERE / "joint_limits.cpp",))
```

- [ ] **Step 2: Run the test and verify the missing source failure**

Run:

```bash
.venv/bin/python -m pytest \
  ramp_up/cpp/01_stl_containers/lessons/02_headers_and_translation_units -q
```

Expected: fail because `joint_limits.cpp` or `solution.cpp` is missing.

- [ ] **Step 3: Add the shared declaration and definition**

Create `joint_limits.hpp`:

```cpp
#pragma once

struct JointLimits {
    double lower;
    double upper;
};

double clamp_to_limits(double command, const JointLimits& limits);
```

Create `joint_limits.cpp`:

```cpp
#include "joint_limits.hpp"

#include <algorithm>
#include <cassert>

double clamp_to_limits(double command, const JointLimits& limits) {
    assert(limits.lower <= limits.upper);
    return std::clamp(command, limits.lower, limits.upper);
}
```

- [ ] **Step 4: Add reference and learner translation units**

Create `solution.cpp`:

```cpp
#include "joint_limits.hpp"

#include <cassert>
#include <iostream>

bool command_is_safe(double command, const JointLimits& limits) {
    return command >= limits.lower && command <= limits.upper;
}

int main() {
    const JointLimits limits{-1.5, 1.5};
    assert(command_is_safe(0.5, limits));
    assert(!command_is_safe(2.0, limits));
    assert(clamp_to_limits(2.0, limits) == 1.5);
    std::cout << "ALL TESTS PASSED\n";
}
```

Create `starter.cpp` with the same includes and `main`, but return `false` from
`command_is_safe` after casting both parameters to `void`.

- [ ] **Step 5: Write the README and verify both modes**

The README must explain declaration versus definition, `#pragma once`,
textual inclusion, independent translation units, and the linker's role. Use
the joint-limit declaration as the robotics application and ask the learner to
predict which file must be recompiled after changing only `solution.cpp`.

Run the reference and practice commands for this lesson. Expected: reference
passes; practice fails at `command_is_safe(0.5, limits)`.

- [ ] **Step 6: Commit the header lesson**

```bash
git add ramp_up/cpp/01_stl_containers/lessons/02_headers_and_translation_units
git commit -m "feat: teach headers and translation units"
```

### Task 5: Micro-Lesson 03 — Values and Copies

**Files:**
- Create: `ramp_up/cpp/01_stl_containers/lessons/03_values_and_copies/README.md`
- Create: `ramp_up/cpp/01_stl_containers/lessons/03_values_and_copies/starter.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/03_values_and_copies/solution.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/03_values_and_copies/test_solution.py`

**Interfaces:**
- Produces: `struct FrameMetadata { int frame_id; std::string source; };`
- Produces: `FrameMetadata with_frame_id(FrameMetadata metadata, int frame_id)`

- [ ] **Step 1: Add the failing lesson test**

Create `test_solution.py`:

```python
from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_values_and_copies_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
```

Run the targeted test. Expected: missing `solution.cpp`.

- [ ] **Step 2: Add the reference program**

Create `solution.cpp`:

```cpp
#include <cassert>
#include <iostream>
#include <string>

struct FrameMetadata {
    int frame_id;
    std::string source;
};

FrameMetadata with_frame_id(FrameMetadata metadata, int frame_id) {
    metadata.frame_id = frame_id;
    return metadata;
}

int main() {
    FrameMetadata original{7, "wrist-camera"};
    FrameMetadata updated = with_frame_id(original, 8);
    assert(original.frame_id == 7);
    assert(updated.frame_id == 8);
    assert(updated.source == "wrist-camera");
    std::cout << "ALL TESTS PASSED\n";
}
```

Create `starter.cpp` with the same struct and `main`, but have
`with_frame_id` cast `frame_id` to `void` and return the unchanged parameter.

- [ ] **Step 3: Write the README**

Explain that a C++ object is a value by default, parameter passing by value
constructs an independent function parameter, and copying a `std::string`
duplicates its value. The application is producing revised frame metadata
without mutating the capture record. Ask learners to predict both frame IDs
before running.

- [ ] **Step 4: Verify and commit**

Run reference mode, practice mode, and the structural contract test. Expected:
reference passes; practice fails because `updated.frame_id` remains `7`.

```bash
git add ramp_up/cpp/01_stl_containers/lessons/03_values_and_copies
git commit -m "feat: teach C++ value semantics"
```

### Task 6: Micro-Lesson 04 — References and Const

**Files:**
- Create: `ramp_up/cpp/01_stl_containers/lessons/04_references_and_const/README.md`
- Create: `ramp_up/cpp/01_stl_containers/lessons/04_references_and_const/starter.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/04_references_and_const/solution.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/04_references_and_const/test_solution.py`

**Interfaces:**
- Produces: `struct JointState { std::vector<double> positions; };`
- Produces: `double max_abs_position(const JointState& state)`
- Produces: `void clamp_in_place(JointState& state, double limit)`

- [ ] **Step 1: Add the lesson test and verify it fails for a missing source**

Use this test body:

```python
from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_references_and_const_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
```

- [ ] **Step 2: Add the reference and learner programs**

Create `solution.cpp`:

```cpp
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

struct JointState {
    std::vector<double> positions;
};

double max_abs_position(const JointState& state) {
    double maximum = 0.0;
    for (double position : state.positions) {
        maximum = std::max(maximum, std::abs(position));
    }
    return maximum;
}

void clamp_in_place(JointState& state, double limit) {
    for (double& position : state.positions) {
        position = std::clamp(position, -limit, limit);
    }
}

int main() {
    JointState state{{-2.0, 0.5, 3.0}};
    assert(max_abs_position(state) == 3.0);
    clamp_in_place(state, 1.0);
    assert((state.positions == std::vector<double>{-1.0, 0.5, 1.0}));
    std::cout << "ALL TESTS PASSED\n";
}
```

Create `starter.cpp` with the same declarations and `main`; return `0.0` from
`max_abs_position` and leave `clamp_in_place` as a no-op after casting its
parameters to `void`.

- [ ] **Step 3: Write the README**

Teach `const T&` as a non-owning read-only alias and `T&` as a non-owning
mutable alias. Explicitly distinguish the referent from the reference syntax.
Use joint-state inspection and safety clamping as the application. Prediction
questions must ask whether caller state changes for `T`, `const T&`, and `T&`.

- [ ] **Step 4: Verify and commit**

Run reference and practice modes. Expected: reference passes; practice fails
at the maximum assertion before reaching the mutation assertion.

```bash
git add ramp_up/cpp/01_stl_containers/lessons/04_references_and_const
git commit -m "feat: teach references and const"
```

### Task 7: Micro-Lesson 05 — Pointer Borrows

**Files:**
- Create: `ramp_up/cpp/01_stl_containers/lessons/05_pointer_borrows/README.md`
- Create: `ramp_up/cpp/01_stl_containers/lessons/05_pointer_borrows/starter.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/05_pointer_borrows/solution.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/05_pointer_borrows/test_solution.py`

**Interfaces:**
- Produces: `struct Sensor { std::string name; double rate_hz; };`
- Produces: `const Sensor* find_sensor(const std::vector<Sensor>& sensors, const std::string& name)`

- [ ] **Step 1: Add the test and verify the missing-source failure**

```python
from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_pointer_borrows_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
```

- [ ] **Step 2: Add the reference and learner programs**

Create `solution.cpp`:

```cpp
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

struct Sensor {
    std::string name;
    double rate_hz;
};

const Sensor* find_sensor(
    const std::vector<Sensor>& sensors,
    const std::string& name
) {
    for (const Sensor& sensor : sensors) {
        if (sensor.name == name) {
            return &sensor;
        }
    }
    return nullptr;
}

int main() {
    const std::vector<Sensor> sensors{
        {"wrist-camera", 30.0},
        {"joint-encoder", 100.0},
    };
    const Sensor* camera = find_sensor(sensors, "wrist-camera");
    assert(camera != nullptr);
    assert(camera->rate_hz == 30.0);
    assert(find_sensor(sensors, "missing") == nullptr);
    std::cout << "ALL TESTS PASSED\n";
}
```

Create `starter.cpp` with the same struct and `main`; have `find_sensor` cast
its parameters to `void` and return `nullptr`.

- [ ] **Step 3: Write the README**

Teach a raw pointer as nullable, non-owning access in modern application code.
State that the returned pointer remains valid only while the vector exists and
is not reallocated. Use a sensor registry lookup as the application. Prediction
questions must cover `nullptr`, `->`, `&sensor`, and vector reallocation.

- [ ] **Step 4: Verify and commit**

Run reference and practice modes. Expected: reference passes; practice fails
at `camera != nullptr`.

```bash
git add ramp_up/cpp/01_stl_containers/lessons/05_pointer_borrows
git commit -m "feat: teach non-owning pointer borrows"
```

### Task 8: Micro-Lesson 06 — Optional Results

**Files:**
- Create: `ramp_up/cpp/01_stl_containers/lessons/06_optional_results/README.md`
- Create: `ramp_up/cpp/01_stl_containers/lessons/06_optional_results/starter.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/06_optional_results/solution.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/06_optional_results/test_solution.py`

**Interfaces:**
- Produces: `struct JointSample { std::int64_t timestamp_ns; double position; };`
- Produces: `std::optional<JointSample> latest_at_or_before(const std::vector<JointSample>& samples, std::int64_t timestamp_ns)`

- [ ] **Step 1: Add the lesson test and verify it fails**

```python
from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_optional_results_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
```

- [ ] **Step 2: Add the reference and learner programs**

Create `solution.cpp`:

```cpp
#include <cassert>
#include <cstdint>
#include <iostream>
#include <optional>
#include <vector>

struct JointSample {
    std::int64_t timestamp_ns;
    double position;
};

std::optional<JointSample> latest_at_or_before(
    const std::vector<JointSample>& samples,
    std::int64_t timestamp_ns
) {
    std::optional<JointSample> result;
    for (const JointSample& sample : samples) {
        if (sample.timestamp_ns <= timestamp_ns) {
            result = sample;
        }
    }
    return result;
}

int main() {
    const std::vector<JointSample> samples{{10, 0.1}, {20, 0.2}, {30, 0.3}};
    const auto found = latest_at_or_before(samples, 25);
    assert(found.has_value());
    assert(found->timestamp_ns == 20);
    assert(!latest_at_or_before(samples, 5).has_value());
    std::cout << "ALL TESTS PASSED\n";
}
```

Create `starter.cpp` with the same struct and `main`; have
`latest_at_or_before` cast its parameters to `void` and return `std::nullopt`.

- [ ] **Step 3: Write the README**

Teach `std::optional<T>` as an owned value that may be absent, distinct from a
borrowed raw pointer. Use timestamp lookup for sensor alignment as the
application. Prediction questions must cover `has_value`, `operator->`,
`std::nullopt`, and whether the returned sample depends on the source vector's
lifetime.

- [ ] **Step 4: Verify and commit**

Run reference and practice modes. Expected: reference passes; practice fails
because the lookup at timestamp `25` has no value.

```bash
git add ramp_up/cpp/01_stl_containers/lessons/06_optional_results
git commit -m "feat: teach optional sensor results"
```

### Task 9: Micro-Lesson 07 — Containers and Contiguous Storage

**Files:**
- Create: `ramp_up/cpp/01_stl_containers/lessons/07_containers_and_storage/README.md`
- Create: `ramp_up/cpp/01_stl_containers/lessons/07_containers_and_storage/starter.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/07_containers_and_storage/solution.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/07_containers_and_storage/test_solution.py`

**Interfaces:**
- Produces: `std::vector<std::uint8_t> make_gray_frame(std::size_t width, std::size_t height, std::uint8_t fill)`
- Produces: `long long checksum(const std::vector<std::uint8_t>& pixels)`

- [ ] **Step 1: Add the lesson test and verify it fails**

```python
from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_containers_and_storage_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
```

- [ ] **Step 2: Add the reference and learner programs**

Create `solution.cpp`:

```cpp
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

std::vector<std::uint8_t> make_gray_frame(
    std::size_t width,
    std::size_t height,
    std::uint8_t fill
) {
    return std::vector<std::uint8_t>(width * height, fill);
}

long long checksum(const std::vector<std::uint8_t>& pixels) {
    long long total = 0;
    for (std::uint8_t pixel : pixels) {
        total += pixel;
    }
    return total;
}

int main() {
    const auto frame = make_gray_frame(4, 3, 7);
    assert(frame.size() == 12);
    assert(frame.front() == 7 && frame.back() == 7);
    assert(checksum(frame) == 84);
    assert(frame.data() + frame.size() == &frame.back() + 1);
    std::cout << "ALL TESTS PASSED\n";
}
```

Create `starter.cpp` with the same declarations and `main`; return an empty
vector from `make_gray_frame` and `0` from `checksum`.

- [ ] **Step 3: Write the README**

Teach `std::vector<T>` as a value that owns contiguous elements and distinguish
the vector object from its element storage. Use a grayscale camera frame as the
application. Prediction questions must cover `size`, `data`, element type,
copying the vector, and iterator/reference invalidation after growth.

- [ ] **Step 4: Verify and commit**

Run reference and practice modes. Expected: reference passes; practice fails
at `frame.size() == 12`.

```bash
git add ramp_up/cpp/01_stl_containers/lessons/07_containers_and_storage
git commit -m "feat: teach contiguous frame storage"
```

### Task 10: Micro-Lesson 08 — Algorithms and Iteration

**Files:**
- Create: `ramp_up/cpp/01_stl_containers/lessons/08_algorithms_and_iteration/README.md`
- Create: `ramp_up/cpp/01_stl_containers/lessons/08_algorithms_and_iteration/starter.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/08_algorithms_and_iteration/solution.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/08_algorithms_and_iteration/test_solution.py`

**Interfaces:**
- Produces: `struct CameraFrame { int frame_id; std::int64_t timestamp_ns; };`
- Produces: `std::vector<int> fresh_frame_ids(const std::vector<CameraFrame>& frames, std::int64_t cutoff_ns)`

- [ ] **Step 1: Add the lesson test and verify it fails**

```python
from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_algorithms_and_iteration_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
```

- [ ] **Step 2: Add the reference and learner programs**

Create `solution.cpp`:

```cpp
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

struct CameraFrame {
    int frame_id;
    std::int64_t timestamp_ns;
};

std::vector<int> fresh_frame_ids(
    const std::vector<CameraFrame>& frames,
    std::int64_t cutoff_ns
) {
    std::vector<int> ids;
    for (const CameraFrame& frame : frames) {
        if (frame.timestamp_ns >= cutoff_ns) {
            ids.push_back(frame.frame_id);
        }
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

int main() {
    const std::vector<CameraFrame> frames{{3, 30}, {1, 10}, {2, 20}};
    assert((fresh_frame_ids(frames, 20) == std::vector<int>{2, 3}));
    assert(fresh_frame_ids(frames, 40).empty());
    std::cout << "ALL TESTS PASSED\n";
}
```

Create `starter.cpp` with the same struct and `main`; cast both function
parameters to `void` and return an empty vector.

- [ ] **Step 3: Write the README**

Teach range-based iteration, `const T&` loop variables, the iterator pair
passed to `std::sort`, and the separation between containers and algorithms.
Use selection of fresh frames as the application. Ask learners to predict
input order, result order, and whether the original vector changes.

- [ ] **Step 4: Verify and commit**

Run reference and practice modes. Expected: reference passes; practice fails
for cutoff `20`.

```bash
git add ramp_up/cpp/01_stl_containers/lessons/08_algorithms_and_iteration
git commit -m "feat: teach algorithms over frame samples"
```

### Task 11: Micro-Lesson 09 — Introductory Templates

**Files:**
- Create: `ramp_up/cpp/01_stl_containers/lessons/09_introductory_templates/README.md`
- Create: `ramp_up/cpp/01_stl_containers/lessons/09_introductory_templates/starter.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/09_introductory_templates/solution.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/09_introductory_templates/test_solution.py`

**Interfaces:**
- Produces: `template <typename T> struct TimedSample { std::int64_t timestamp_ns; T value; };`
- Produces: `template <typename T> const TimedSample<T>* latest_sample(const std::vector<TimedSample<T>>& samples)`

- [ ] **Step 1: Add the lesson test and verify it fails**

```python
from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_introductory_templates_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
```

- [ ] **Step 2: Add the reference and learner programs**

Create `solution.cpp`:

```cpp
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

template <typename T>
struct TimedSample {
    std::int64_t timestamp_ns;
    T value;
};

template <typename T>
const TimedSample<T>* latest_sample(
    const std::vector<TimedSample<T>>& samples
) {
    if (samples.empty()) {
        return nullptr;
    }
    return &samples.back();
}

int main() {
    const std::vector<TimedSample<double>> joints{{10, 0.1}, {20, 0.2}};
    const std::vector<TimedSample<std::string>> modes{{15, "idle"}};
    const auto* latest_joint = latest_sample(joints);
    const auto* latest_mode = latest_sample(modes);
    assert(latest_joint != nullptr);
    assert(latest_mode != nullptr);
    assert(latest_joint->value == 0.2);
    assert(latest_mode->value == "idle");
    assert(latest_sample(std::vector<TimedSample<int>>{}) == nullptr);
    std::cout << "ALL TESTS PASSED\n";
}
```

Create `starter.cpp` with the same template struct and `main`; have
`latest_sample` cast `samples` to `void` and return `nullptr`.

- [ ] **Step 3: Write the README**

Teach a template as a compile-time recipe instantiated for concrete types.
Explain why template definitions must be visible where instantiated. Use one
timestamp wrapper for joint positions and robot modes as the application.
Prediction questions must identify the concrete instantiated types and explain
why no runtime type check is needed.

- [ ] **Step 4: Verify and commit**

Run reference and practice modes. Expected: reference passes; practice fails
at `latest_joint != nullptr` without dereferencing the null pointer or hanging.

```bash
git add ramp_up/cpp/01_stl_containers/lessons/09_introductory_templates
git commit -m "feat: introduce typed sensor templates"
```

### Task 12: Micro-Lesson 10 — Classes and Member Initialization

**Files:**
- Create: `ramp_up/cpp/01_stl_containers/lessons/10_classes_and_member_initialization/README.md`
- Create: `ramp_up/cpp/01_stl_containers/lessons/10_classes_and_member_initialization/starter.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/10_classes_and_member_initialization/solution.cpp`
- Create: `ramp_up/cpp/01_stl_containers/lessons/10_classes_and_member_initialization/test_solution.py`

**Interfaces:**
- Produces class: `CameraFrame`
- Produces constructor: `CameraFrame(std::size_t width, std::size_t height, std::uint8_t fill)`
- Produces const accessors: `width()`, `height()`, `pixel_count()`, `checksum()`

- [ ] **Step 1: Add the lesson test and verify it fails**

```python
from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_classes_and_member_initialization_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
```

- [ ] **Step 2: Add the reference program**

Create `solution.cpp`:

```cpp
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

class CameraFrame {
public:
    CameraFrame(std::size_t width, std::size_t height, std::uint8_t fill)
        : width_(width),
          height_(height),
          pixels_(width * height, fill) {}

    std::size_t width() const { return width_; }
    std::size_t height() const { return height_; }
    std::size_t pixel_count() const { return pixels_.size(); }

    long long checksum() const {
        long long total = 0;
        for (std::uint8_t pixel : pixels_) {
            total += pixel;
        }
        return total;
    }

private:
    std::size_t width_;
    std::size_t height_;
    std::vector<std::uint8_t> pixels_;
};

int main() {
    const CameraFrame frame(4, 3, 7);
    assert(frame.width() == 4);
    assert(frame.height() == 3);
    assert(frame.pixel_count() == 12);
    assert(frame.checksum() == 84);
    std::cout << "ALL TESTS PASSED\n";
}
```

Create `starter.cpp` with the same class shape and `main`. Initialize `width_`
and `height_` to zero and `pixels_` empty; return the stored members from the
accessors and keep the checksum loop. The exercise is to repair only the
member-initializer list.

- [ ] **Step 3: Write the README**

Teach declaration order, access control, constructors, member-initializer
lists, and `const` member functions as one class-construction mental model.
Emphasize that `std::vector` already owns and releases its storage; this class
needs no destructor and previews the Rule of Zero. Use a camera frame as the
application.

- [ ] **Step 4: Verify and commit**

Run reference and practice modes. Expected: reference passes; practice fails at
`frame.width() == 4`.

```bash
git add ramp_up/cpp/01_stl_containers/lessons/10_classes_and_member_initialization
git commit -m "feat: teach class member initialization"
```

### Task 13: Module 01 Diagnostic and Preserved Capstone

**Files:**
- Rewrite: `ramp_up/cpp/01_stl_containers/README.md`
- Modify: `ramp_up/cpp/01_stl_containers/starter.cpp`
- Modify: `ramp_up/cpp/01_stl_containers/solution.cpp`
- Modify: `tests/test_cpp_curriculum_structure.py`

**Interfaces:**
- Preserves capstone functions:
  - `std::string reverseWords(const std::string& text)`
  - `std::unordered_map<char, int> charFrequencies(const std::string& text)`
  - `std::vector<int> topKSmallest(std::vector<int> values, std::size_t k)`
  - `std::map<int, std::vector<std::string>> groupByLength(const std::vector<std::string>& words)`
  - `long long sumOfUnique(const std::vector<int>& values)`
- Produces exact Module 01 lesson inventory for structural validation

- [ ] **Step 1: Extend the structural tests with the exact lesson inventory and neutral module scan**

Append to `tests/test_cpp_curriculum_structure.py`:

```python
MODULE_01 = CPP_ROOT / "01_stl_containers"
MODULE_01_LESSONS = (
    "01_build_compile_and_link",
    "02_headers_and_translation_units",
    "03_values_and_copies",
    "04_references_and_const",
    "05_pointer_borrows",
    "06_optional_results",
    "07_containers_and_storage",
    "08_algorithms_and_iteration",
    "09_introductory_templates",
    "10_classes_and_member_initialization",
)


def test_module_01_has_the_approved_micro_lesson_inventory() -> None:
    lessons_dir = MODULE_01 / "lessons"
    actual = tuple(sorted(path.name for path in lessons_dir.iterdir() if path.is_dir()))
    assert actual == MODULE_01_LESSONS


def test_module_01_is_source_language_neutral() -> None:
    forbidden = ("java", "java:", "arraylist", "hashmap", "treemap")
    violations = []
    for path in sorted(MODULE_01.rglob("*")):
        if path.is_file() and path.suffix in {".md", ".cpp", ".py"}:
            text = path.read_text().lower()
            for phrase in forbidden:
                if phrase in text:
                    violations.append(f"{path.relative_to(ROOT)}: {phrase}")
    assert violations == []
```

- [ ] **Step 2: Run the structural test and verify current capstone wording fails**

Run:

```bash
.venv/bin/python -m pytest \
  tests/test_cpp_curriculum_structure.py::test_module_01_is_source_language_neutral -q
```

Expected: fail with current README and C++ comment locations.

- [ ] **Step 3: Replace the module README with an index and diagnostic**

Rewrite `01_stl_containers/README.md` with:

- module purpose: read and write ordinary C++ values and standard containers;
- prerequisites: general programming experience only;
- ordered table of all ten micro-lessons, each with concept and application;
- diagnostic questions:
  1. distinguish compile and link errors;
  2. predict a by-value copy;
  3. choose `const T&` versus `T&`;
  4. explain a returned raw pointer's lifetime;
  5. distinguish pointer absence from `std::optional` value absence;
  6. identify contiguous frame bytes;
  7. explain template instantiation;
  8. trace a class member-initializer list;
- fast-path instruction: attempt the root capstone if every diagnostic answer is
  explainable without guessing;
- unchanged reference and practice capstone commands;
- the five capstone contracts and expected complexity;
- links back to `../../README.md` and forward to
  `../02_ownership_and_raii/`.

- [ ] **Step 4: Neutralize capstone comments without changing behavior**

In `starter.cpp` and `solution.cpp`:

- change compile commands from `-std=c++17` to `-std=c++20`;
- remove every source-language label or library comparison;
- retain the C++ contract, hints, assertions, and function signatures;
- explain by-value, reference, map insertion, structured bindings, and
  `long long` directly in C++;
- do not alter executable behavior.

- [ ] **Step 5: Run capstone, structural, and practice-selection checks**

Run:

```bash
.venv/bin/python -m pytest \
  ramp_up/cpp/01_stl_containers \
  tests/test_cpp_curriculum_structure.py -q
PRACTICE=1 .venv/bin/python -m pytest \
  ramp_up/cpp/01_stl_containers/test_solution.py -q
```

Expected: reference and structural tests pass; practice mode fails because the
capstone stubs remain incomplete.

- [ ] **Step 6: Commit the Module 01 capstone migration**

```bash
git add ramp_up/cpp/01_stl_containers tests/test_cpp_curriculum_structure.py
git commit -m "docs: turn STL drill into Module 01 capstone"
```

### Task 14: Foundation and Module 01 Acceptance

**Files:**
- Modify only if verification exposes a defect:
  - `ramp_up/cpp/cpp_test_support.py`
  - `ramp_up/cpp/conftest.py`
  - `ramp_up/cpp/README.md`
  - `ramp_up/cpp/CONCEPT_MAP.md`
  - `ramp_up/cpp/LEARNING_POINTS.md`
  - `ramp_up/cpp/01_stl_containers/**`
  - `tests/test_cpp_test_support.py`
  - `tests/test_cpp_curriculum_structure.py`

**Interfaces:**
- Verifies all interfaces produced by Tasks 1–13
- Produces a green reference suite and intentionally failing Module 01 practice suite

- [ ] **Step 1: Run formatting and placeholder checks**

Run:

```bash
git diff --check
rg -n 'coming from Java|Java-to-C\\+\\+|Java→C\\+\\+|JAVA:' \
  README.md ramp_up/cpp/README.md ramp_up/cpp/CONCEPT_MAP.md \
  ramp_up/cpp/LEARNING_POINTS.md ramp_up/cpp/01_stl_containers
```

Expected: `git diff --check` exits zero and `rg` finds no matches.

- [ ] **Step 2: Run the focused foundation suite**

Run:

```bash
.venv/bin/python -m pytest \
  tests/test_cpp_test_support.py \
  tests/test_cpp_curriculum_structure.py \
  ramp_up/cpp/01_stl_containers -q
```

Expected: all focused reference tests pass.

- [ ] **Step 3: Run every C++ capstone and micro-lesson**

Run:

```bash
.venv/bin/python -m pytest ramp_up/cpp -q
```

Expected: all six capstones and all ten Module 01 micro-lessons pass in
reference mode.

- [ ] **Step 4: Prove every Module 01 starter is selected and incomplete**

Run each command separately:

```bash
PRACTICE=1 .venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/01_build_compile_and_link -q
PRACTICE=1 .venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/02_headers_and_translation_units -q
PRACTICE=1 .venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/03_values_and_copies -q
PRACTICE=1 .venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/04_references_and_const -q
PRACTICE=1 .venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/05_pointer_borrows -q
PRACTICE=1 .venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/06_optional_results -q
PRACTICE=1 .venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/07_containers_and_storage -q
PRACTICE=1 .venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/08_algorithms_and_iteration -q
PRACTICE=1 .venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/09_introductory_templates -q
PRACTICE=1 .venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/10_classes_and_member_initialization -q
```

Expected: every command fails quickly at its documented behavioral check; none
times out and none accidentally runs `solution.cpp`.

- [ ] **Step 5: Run the complete repository suite**

Run:

```bash
.venv/bin/python -m pytest -q
```

Expected: the complete reference suite passes.

- [ ] **Step 6: Review the public diff**

Run:

```bash
git status --short
git diff --stat
git diff -- README.md ramp_up/cpp tests/test_cpp_test_support.py \
  tests/test_cpp_curriculum_structure.py
```

Expected: only the approved curriculum foundation, Module 01, and test-harness
changes are present; no Python ramp-up or interview-question behavior changed.

- [ ] **Step 7: Commit acceptance fixes if verification required changes**

If Steps 1–6 required corrections:

```bash
git add README.md ramp_up/cpp tests/test_cpp_test_support.py \
  tests/test_cpp_curriculum_structure.py
git commit -m "test: verify C++ curriculum foundation"
```

If no corrections were required, do not create an empty commit.

## Plan Completion Criteria

This plan is complete when:

- the shared runner compiles every C++ lesson and capstone as C++20;
- the four public C++ entry documents are language-neutral;
- all ten Module 01 micro-lessons satisfy the approved lesson contract;
- every micro-lesson has a robotics or inference application;
- Module 01's original exercise remains available as the root capstone;
- reference mode passes for the entire repository;
- every Module 01 practice starter is demonstrably selected and fails quickly;
- the branch contains no unrelated changes.
