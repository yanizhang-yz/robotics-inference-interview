# C++ Modules 02–06 Micro-Lessons Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Expand the C++ ramp-up from one split module into a six-module, interview-aware curriculum whose drills can be started directly from each `starter.cpp`.

**Architecture:** Keep the six existing top-level modules and their current capstone behavior. Add one independently compilable directory per concept under modules 02–06, use `main()` assertions as each C++ lesson's behavioral test, and delegate compilation to the shared pytest fixture. Retrofit module 01 and all six capstones with self-contained drill briefs, while module READMEs become short navigation and diagnostic pages.

**Tech Stack:** C++20, standard library only, pytest, the existing `ramp_up/cpp/cpp_test_support.py` compiler harness, `clang++` or `g++`.

## Global Constraints

- Do not modify any file under `questions/`.
- Do not create top-level C++ modules 07, 08, or 09; this curriculum has six modules.
- Compile every lesson with `-std=c++20 -Wall -Wextra -Werror=return-type`.
- Add `-pthread` only to the threading lessons in module 06.
- Every successful C++ program must print `ALL TESTS PASSED` as its final non-empty line.
- The exact public module directory set is modules 01–06; reject every extra
  top-level `[0-9][0-9]_*` directory.
- Every lesson directory contains exactly `README.md`, `starter.cpp`,
  `solution.cpp`, and `test_solution.py`. The only support-file whitelist entry
  is module-01 lesson 02's `joint_limits.hpp` and `joint_limits.cpp`;
  `__pycache__` is ignored as generated state.
- Every lesson README contains these level-two headings in order: `The problem this lesson solves`, `The lesson`, `How interviewers test this`, `Muscle memory`, `The drills`, and `How to practice`.
- Every lesson introduces one primary C++ concept, connects it to a robotics or
  inference scenario, and gives explicit Prediction, Implementation, Follow-up,
  and Evidence probes in `How interviewers test this`.
- Every `starter.cpp` begins with one leading comment block whose first content
  is `DRILL BRIEF`, followed in order by `Concept:`, `Scenario:`, `Implement:`,
  `Behavior:`, `Interview focus:`, `Tests:`, `Run:`, and `Done when:`.
- Every new lesson and capstone `Behavior:` block includes a concrete `Example:`
  and an important `Edge:` case.
- Starter comments specify observable behavior and edge cases without containing a completed algorithm or copy-pasteable solution.
- Starter stubs compile and fail an assertion quickly; concurrency starters must never wait indefinitely when incomplete.
- Preserve the learner's existing implementations in module-01 lesson starters
  01–03 and the untracked `.drill/` state by working only in the isolated
  worktree. In the isolated branch, leave all ten starter function bodies
  unchanged from their committed baseline and edit only comments.
- The module-01 capstone stays in `01_stl_containers/capstone/`; module 02–06 capstones stay at their existing module roots.
- Memory-layout correctness tests never depend on timing ratios.
- Concurrency tests use deterministic, race-free in-protocol condition-variable
  handshakes and thread joins. They never use sleeps, yield polling, fixed poll
  counts, or scheduler-dependent proof.
- Every module-02–06 capstone reference or practice command targets that
  module's root `test_solution.py`, never the recursively collected directory.

## File and Content Map

The shared contract test owns the exact module tree, exact lesson file sets,
inventory, headings, interview probes, navigation, capstone-only commands, and
ordered leading drill briefs. Each lesson's `test_solution.py` owns compilation
of that lesson through `run_cpp_lesson`. Each C++ `main()` owns the
lesson-specific behavioral assertions. Each module README owns its Concept and
Application lesson table, prerequisite diagnostics, capstone positioning,
reference and learner commands, and previous/next navigation; concept teaching
lives in lesson READMEs.

For a normal lesson test, use this exact shape with the function name from the lesson specification below:

```python
from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_scope_and_lifetime_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
```

For module-06 lessons, the call is `run_cpp_lesson(HERE, extra_flags=("-pthread",), timeout=10)`.

Every starter's first comment block follows this exact field order. This
concrete example is the header for module-02 lesson 01:

```cpp
/*
DRILL BRIEF
Concept: Local objects are destroyed at scope exit in reverse construction order.
Scenario: Trace nested sensor-session helpers that must clean up predictably.
Implement: scope_trace().
Behavior: Record the required lifetime trace.
Example: outer, inner-a, and inner-b construct before reverse inner cleanup.
Edge: after-inner occurs before outer destruction.
Interview focus: Predict the exact lifetime trace and name each scope boundary.
Tests: main() compares every construction, ordinary event, and destruction entry in order.
Run: PRACTICE=1 uv run pytest ramp_up/cpp/02_ownership_and_raii/lessons/01_scope_and_lifetime -q
Done when: The final output line is ALL TESTS PASSED.
*/
```

Every other starter keeps these labels and takes its exact content from the
lesson specification in its module task.

---

### Task 1: Enforce the Complete Curriculum Contract

**Files:**
- Modify: `tests/test_cpp_curriculum_structure.py`
- Test: `tests/test_cpp_curriculum_structure.py`

**Interfaces:**
- Consumes: `CPP_ROOT`, `_level_two_sections()`, and the current module-01 contract constants.
- Produces: `MODULE_LESSONS`, `CONTRACT_HEADINGS`, and starter-brief validation used as the acceptance gate for Tasks 2–7.

- [ ] **Step 1: Extend the required README headings**

Set `CONTRACT_HEADINGS` to this exact tuple:

```python
CONTRACT_HEADINGS = (
    "## The problem this lesson solves",
    "## The lesson",
    "## How interviewers test this",
    "## Muscle memory",
    "## The drills",
    "## How to practice",
)
```

- [ ] **Step 2: Replace the module-01-only inventory with all six approved inventories**

Add this exact mapping and parameterized test:

```python
import pytest


MODULE_LESSONS = {
    "01_stl_containers": (
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
    ),
    "02_ownership_and_raii": (
        "01_scope_and_lifetime",
        "02_construction_and_destruction",
        "03_owners_and_borrowers",
        "04_unique_ptr",
        "05_ownership_transfer",
        "06_span_views",
        "07_raii_resources",
    ),
    "03_move_semantics_rule_of_five": (
        "01_observing_copy_cost",
        "02_temporaries_and_value_categories",
        "03_move_construction",
        "04_moved_from_state",
        "05_copy_elision",
        "06_noexcept_moves",
        "07_rule_of_zero",
        "08_rule_of_five",
    ),
    "04_virtual_functions_and_vtables": (
        "01_interfaces_and_dynamic_dispatch",
        "02_virtual_and_override",
        "03_virtual_destructors",
        "04_polymorphic_ownership",
        "05_object_slicing",
        "06_dispatch_cost",
    ),
    "05_memory_layout_and_cache": (
        "01_storage_and_object_layout",
        "02_alignment_and_padding",
        "03_cache_lines_and_locality",
        "04_traversal_and_contiguous_storage",
        "05_aos_and_soa",
        "06_allocation_and_reserve",
    ),
    "06_threads_atomics_queues": (
        "01_thread_lifetime",
        "02_lambda_captures",
        "03_data_races_and_mutexes",
        "04_raii_locks",
        "05_atomics",
        "06_condition_variables",
        "07_bounded_queues",
        "08_clean_shutdown",
    ),
}


@pytest.mark.parametrize("module_name, expected", MODULE_LESSONS.items())
def test_module_has_approved_micro_lesson_inventory(
    module_name: str, expected: tuple[str, ...]
) -> None:
    lessons_dir = CPP_ROOT / module_name / "lessons"
    actual = (
        tuple(sorted(path.name for path in lessons_dir.iterdir() if path.is_dir()))
        if lessons_dir.exists()
        else ()
    )
    assert actual == expected
```

Remove the superseded `MODULE_01`, `MODULE_01_LESSONS`, and module-01-only inventory test.

- [ ] **Step 2a: Enforce the exact public tree and file sets**

Assert that the sorted top-level `[0-9][0-9]_*` directory set is exactly the
six `MODULE_LESSONS` keys. For every lesson, assert the exact four contract
files plus the explicit module-01 lesson-02 `joint_limits.hpp/.cpp` whitelist;
ignore only generated `__pycache__` directories.

- [ ] **Step 3: Add ordered leading starter drill-brief validation for all lessons and capstones**

Add this exact test:

```python
DRILL_BRIEF_FIELDS = (
    "DRILL BRIEF",
    "Concept:",
    "Scenario:",
    "Implement:",
    "Behavior:",
    "Interview focus:",
    "Tests:",
    "Run:",
    "Done when:",
)


def test_all_starters_have_self_contained_drill_briefs() -> None:
    starters = [
        CPP_ROOT / module_name / "lessons" / lesson_name / "starter.cpp"
        for module_name, lesson_names in MODULE_LESSONS.items()
        for lesson_name in lesson_names
    ]
    starters.extend(
        [CPP_ROOT / "01_stl_containers" / "capstone" / "starter.cpp"]
        + [
            CPP_ROOT / module_name / "starter.cpp"
            for module_name in tuple(MODULE_LESSONS)[1:]
        ]
    )
    violations = []
    for starter in starters:
        if not starter.is_file():
            violations.append(f"{starter.relative_to(ROOT)}: missing starter.cpp")
            continue
        comment_lines = _leading_comment_lines(starter.read_text())
        if not comment_lines or comment_lines[0] != "DRILL BRIEF":
            violations.append(
                f"{starter.relative_to(ROOT)}: DRILL BRIEF must start the "
                "leading comment block"
            )
            continue
        positions = [
            next(
                index
                for index, line in enumerate(comment_lines)
                if line == field or line.startswith(field)
            )
            for field in DRILL_BRIEF_FIELDS
        ]
        if positions != sorted(positions):
            violations.append(
                f"{starter.relative_to(ROOT)}: drill-brief fields are out of order"
            )
    assert violations == []
```

Add companion structural tests that require all 35 new lesson interview
sections to contain Prediction, Implementation, Follow-up, and Evidence probes;
require each new-lesson and capstone `Behavior:` block to contain `Example:`
and `Edge:`; validate every next-lesson/capstone link; validate each module's
Concept / Application table and previous/next navigation; and validate exact
capstone-only `test_solution.py` commands. Reject sleep/yield polling in module
06 lessons 06–08.

- [ ] **Step 4: Run the structural test and verify the new contract fails for the intended reasons**

Run:

```bash
uv run pytest tests/test_cpp_curriculum_structure.py -q
```

Expected: FAIL because modules 02–06 have no `lessons/` inventories, module-01 READMEs lack `How interviewers test this`, and existing starters lack the complete drill brief. There must be no syntax or collection error.

- [ ] **Step 5: Commit the contract test**

```bash
git add tests/test_cpp_curriculum_structure.py
git commit -m "test: define complete C++ micro-lesson contract"
```

### Task 2: Retrofit Module 01 Without Losing Learner Work

**Files:**
- Modify: `ramp_up/cpp/01_stl_containers/lessons/*/README.md`
- Modify: `ramp_up/cpp/01_stl_containers/lessons/*/starter.cpp`
- Modify: `ramp_up/cpp/01_stl_containers/capstone/starter.cpp`
- Test: `tests/test_cpp_curriculum_structure.py`
- Test: `ramp_up/cpp/01_stl_containers/lessons/*/test_solution.py`
- Test: `ramp_up/cpp/01_stl_containers/capstone/test_solution.py`

**Interfaces:**
- Consumes: the existing ten lesson drills, current learner edits in lesson starters 01–03, and the drill-brief fields from Task 1.
- Produces: the canonical editor-first experience that all new lessons follow.

- [ ] **Step 1: Record the isolated branch's starter bodies before editing**

Run:

```bash
git diff -- ramp_up/cpp/01_stl_containers/lessons/01_build_compile_and_link/starter.cpp ramp_up/cpp/01_stl_containers/lessons/02_headers_and_translation_units/starter.cpp ramp_up/cpp/01_stl_containers/lessons/03_values_and_copies/starter.cpp
```

Expected: the isolated branch has the committed exercise stubs, while the
learner's answers remain only in the protected primary checkout. Save the
three isolated function bodies in the task report and do not change them.

- [ ] **Step 2: Add the dedicated interview section to all ten READMEs**

Insert `## How interviewers test this` between `## The lesson` and `## Muscle memory`. Each section contains one prediction probe, one implementation probe, and one follow-up probe tied to the existing drill:

| Lesson | Prediction | Implementation | Follow-up |
| --- | --- | --- | --- |
| 01 build/compile/link | classify compiler versus linker failures | repair `clamp_joint_command` | explain which build stage reports each failure |
| 02 headers/TUs | predict duplicate versus missing symbols | implement inclusive limit checking | explain declaration, definition, and ODR placement |
| 03 values/copies | predict which object changes | return revised metadata by value | explain copy cost and when value semantics help |
| 04 references/const | predict copy versus alias | read by `const&`, mutate by `&` | justify a parameter signature |
| 05 pointer borrows | identify a dangling result | return matching sensor or `nullptr` | state the pointee's required lifetime |
| 06 optional results | distinguish absence from a sentinel | return latest eligible sample | compare `optional<T>` with `T*` ownership and lifetime |
| 07 containers/storage | predict vector size and address relationships | allocate and checksum a frame | explain contiguity and reallocation invalidation |
| 08 algorithms/iteration | trace half-open ranges and sort mutation | filter then sort frame IDs | give complexity and comparator follow-ups |
| 09 templates | identify the instantiated types | find latest typed sample | explain compile-time reuse and error expansion |
| 10 classes/initialization | predict member initialization order | build a valid camera frame | explain why declaration order beats initializer-list order |

- [ ] **Step 3: Add lesson-specific drill briefs to the ten starters**

Use the existing function names and assertions as the authoritative behavior. The `Implement:` line names only the current learner-editable functions or constructor. The `Behavior:` line includes the concrete examples already asserted in `main()`. The `Interview focus:` line is the implementation probe from Step 2. Do not alter includes, declarations, function bodies, or assertions.

- [ ] **Step 4: Add a capstone drill brief**

At the top of `capstone/starter.cpp`, describe the three telemetry stages, exact fixture-driven input/output contract, malformed-line behavior, stage selection through `argv[1]`, the capstone practice command, and the final success signal. Do not change the capstone implementation or fixtures.

- [ ] **Step 5: Verify the isolated starter bodies remained unchanged**

Run `git diff --word-diff=porcelain` for the ten lesson starters and inspect
every hunk. Expected: only comment blocks changed; no include, declaration,
function body, or assertion changed. Separately verify the primary checkout
still contains the learner's three answers and `.drill/` state.

- [ ] **Step 6: Run module-01 reference and structural tests**

Run:

```bash
uv run pytest ramp_up/cpp/01_stl_containers -q
uv run pytest tests/test_cpp_curriculum_structure.py -q
```

Expected: all module-01 reference tests PASS. The structural suite still FAILS only for absent modules 02–06 and their capstone briefs.

- [ ] **Step 7: Commit the module-01 retrofit files**

Because this branch is isolated from the primary checkout's learner answers,
stage all ten README and starter comment changes plus the capstone starter.

```bash
git add ramp_up/cpp/01_stl_containers/lessons/*/README.md ramp_up/cpp/01_stl_containers/capstone/starter.cpp
git add ramp_up/cpp/01_stl_containers/lessons/*/starter.cpp
git commit -m "docs: make module 01 drills editor-first"
```

### Task 3: Split Module 02 — Ownership and RAII

**Files:**
- Create: `ramp_up/cpp/02_ownership_and_raii/lessons/01_scope_and_lifetime/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/02_ownership_and_raii/lessons/02_construction_and_destruction/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/02_ownership_and_raii/lessons/03_owners_and_borrowers/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/02_ownership_and_raii/lessons/04_unique_ptr/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/02_ownership_and_raii/lessons/05_ownership_transfer/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/02_ownership_and_raii/lessons/06_span_views/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/02_ownership_and_raii/lessons/07_raii_resources/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Modify: `ramp_up/cpp/02_ownership_and_raii/README.md`
- Modify: `ramp_up/cpp/02_ownership_and_raii/starter.cpp`
- Test: `tests/test_cpp_curriculum_structure.py`

**Interfaces:**
- Consumes: module-01 values, references, pointer borrows, classes, and the shared `run_cpp_lesson` fixture.
- Produces: ownership/lifetime concepts required by module 03 and the unchanged module-02 capstone behavior.

**Exact lesson specifications:**

| Lesson | Required interface and assertions | Mental model, application, and interview focus |
| --- | --- | --- |
| 01 scope/lifetime | `std::vector<std::string> scope_trace()` uses a provided `ScopeMarker`; assert construction `outer`, `inner-a`, `inner-b`, reverse destruction, and an `after-inner` event | Locals die at scope exit in reverse construction order; trace nested sensor-session scopes; predict exact lifetime events. |
| 02 construction/destruction | `PipelineTrace` owns `TracePart capture_` then `TracePart inference_`; assert members construct in declaration order and destruct in reverse, even if initializer text is reordered | An object's lifetime includes its members; reason about camera pipeline dependencies and declaration order. |
| 03 owners/borrowers | `Frame* find_frame(std::vector<Frame>&, int)` and `long long checksum(const Frame&)`; assert match aliases vector storage, missing returns `nullptr`, checksum does not mutate, and no deletion occurs | Containers own elements; references/pointers borrow; identify owner, borrower, nullability, and invalidation boundary. |
| 04 unique_ptr | `std::unique_ptr<Frame> make_frame(std::size_t, std::size_t, std::uint8_t)`; assert non-null ownership, dimensions, pixel count, checksum, and automatic cleanup through a live-object counter | `unique_ptr` is exclusive ownership with destructor cleanup; build a camera-frame factory and explain why copying is disabled. |
| 05 ownership transfer | `std::unique_ptr<Frame> relay_frame(std::unique_ptr<Frame>)` and `long long consume_frame(std::unique_ptr<Frame>)`; assert `std::move` makes the source null, relay preserves address, and consume returns checksum | Moving a `unique_ptr` transfers responsibility; trace a frame handed from capture to inference. |
| 06 span views | `void scale_in_place(std::span<float>, float)` and `double mean(std::span<const float>)`; assert mutation of a middle `subspan`, unchanged surrounding values, whole-range mean, and empty mean `0.0` | `span` borrows contiguous storage and carries length; choose mutable versus const tensor views and state lifetime limits. |
| 07 RAII resources | `ScopedDevice(bool&)`, deleted copy operations, and `int read_with_device(bool&, bool)`; assert the flag is true during the read and false after normal return and after a thrown `std::runtime_error` | Put cleanup in a destructor so every exit path releases a device; explain exception safety and why explicit cleanup is fragile. |

Use these reference implementation kernels; each lesson defines only the
types it needs and wraps its kernel with the assertions listed above:

```cpp
class ScopeMarker {
public:
    ScopeMarker(std::string name, std::vector<std::string>& events)
        : name_(std::move(name)), events_(events) {
        events_.push_back("construct " + name_);
    }
    ~ScopeMarker() { events_.push_back("destroy " + name_); }
private:
    std::string name_;
    std::vector<std::string>& events_;
};

std::vector<std::string> scope_trace() {
    std::vector<std::string> events;
    {
        ScopeMarker outer("outer", events);
        {
            ScopeMarker inner_a("inner-a", events);
            ScopeMarker inner_b("inner-b", events);
        }
        events.push_back("after-inner");
    }
    return events;
}

struct TracePart {
    TracePart(std::string name, std::vector<std::string>& events)
        : name(std::move(name)), events(events) {
        events.push_back("construct " + this->name);
    }
    ~TracePart() { events.push_back("destroy " + name); }
    std::string name;
    std::vector<std::string>& events;
};

class PipelineTrace {
public:
    explicit PipelineTrace(std::vector<std::string>& events)
        : inference_("inference", events), capture_("capture", events) {}
private:
    TracePart capture_;
    TracePart inference_;
};

Frame* find_frame(std::vector<Frame>& frames, int id) {
    for (Frame& frame : frames) if (frame.id == id) return &frame;
    return nullptr;
}

long long checksum(const Frame& frame) {
    long long total = 0;
    for (std::uint8_t pixel : frame.pixels) total += pixel;
    return total;
}

std::unique_ptr<Frame> make_frame(
    std::size_t width, std::size_t height, std::uint8_t fill
) {
    return std::make_unique<Frame>(width, height, fill);
}

std::unique_ptr<Frame> relay_frame(std::unique_ptr<Frame> frame) {
    return frame;
}

long long consume_frame(std::unique_ptr<Frame> frame) {
    return checksum(*frame);
}

void scale_in_place(std::span<float> values, float gain) {
    for (float& value : values) value *= gain;
}

double mean(std::span<const float> values) {
    if (values.empty()) return 0.0;
    double total = 0.0;
    for (float value : values) total += value;
    return total / static_cast<double>(values.size());
}

class ScopedDevice {
public:
    explicit ScopedDevice(bool& active) : active_(active) { active_ = true; }
    ~ScopedDevice() { active_ = false; }
    ScopedDevice(const ScopedDevice&) = delete;
    ScopedDevice& operator=(const ScopedDevice&) = delete;
private:
    bool& active_;
};

int read_with_device(bool& active, bool throw_after_open) {
    ScopedDevice device(active);
    if (throw_after_open) throw std::runtime_error("sensor read failed");
    return active ? 42 : 0;
}
```

- [ ] **Step 1: Add seven compiling lesson tests before their solutions**

Create the seven `test_solution.py` files using function names `test_scope_and_lifetime_lesson`, `test_construction_and_destruction_lesson`, `test_owners_and_borrowers_lesson`, `test_unique_ptr_lesson`, `test_ownership_transfer_lesson`, `test_span_views_lesson`, and `test_raii_resources_lesson`.

- [ ] **Step 2: Run the module lesson tests and observe the missing-source failure**

Run:

```bash
uv run pytest ramp_up/cpp/02_ownership_and_raii/lessons -q
```

Expected: collection reaches each test and reports missing `solution.cpp`.
This is only a wiring check, not the TDD red state.

- [ ] **Step 3: Add minimal compilable reference skeletons and verify behavioral RED**

Create each `solution.cpp` with the required declarations and `main()`
assertions but neutral implementations: empty vectors/owners, zero numeric
results, `nullptr` borrows, and omitted state changes. Run the seven lesson
tests again. Expected: every program compiles and fails its first behavioral
assertion; there are no missing-file or compiler errors.

- [ ] **Step 4: Implement lessons 01–03**

For each lesson, write the six required README sections, the self-contained starter brief and fast-failing stubs, and the complete reference program matching the exact interface/assertions table. Keep each drill under roughly 100 lines of C++ and each README focused on its single concept.

- [ ] **Step 5: Verify lessons 01–03**

```bash
uv run pytest ramp_up/cpp/02_ownership_and_raii/lessons/01_scope_and_lifetime ramp_up/cpp/02_ownership_and_raii/lessons/02_construction_and_destruction ramp_up/cpp/02_ownership_and_raii/lessons/03_owners_and_borrowers -q
```

Expected: 3 PASS.

- [ ] **Step 6: Implement lessons 04–07**

Write all four contract files per lesson using the exact interfaces and assertions above. Include `<memory>` for lessons 04–05, `<span>` for lesson 06, and `<stdexcept>` for lesson 07. Stubs return empty owners, neutral numeric values, or omit state changes so practice tests fail assertions rather than crash.

- [ ] **Step 7: Verify lessons 04–07**

```bash
uv run pytest ramp_up/cpp/02_ownership_and_raii/lessons/04_unique_ptr ramp_up/cpp/02_ownership_and_raii/lessons/05_ownership_transfer ramp_up/cpp/02_ownership_and_raii/lessons/06_span_views ramp_up/cpp/02_ownership_and_raii/lessons/07_raii_resources -q
```

Expected: 4 PASS.

- [ ] **Step 8: Replace the monolithic module README with the index and diagnostic**

The lesson table uses Concept and Application columns for the seven lessons.
The fast-path questions require the learner to predict destruction order,
identify owner versus borrower, trace a `unique_ptr` move, state a span's
lifetime requirement, and explain exception cleanup. Include previous-module
and next-module navigation. Document reference and learner capstone commands
against `ramp_up/cpp/02_ownership_and_raii/test_solution.py` only.

- [ ] **Step 9: Add the module-02 capstone drill brief without changing capstone code**

Name the existing `Buffer`, `makeBuffer`, `moveBuffer`, and `ScopedLogger` work;
state concrete asserted inputs, outputs, and edge cases without giving the
implementation; use `PRACTICE=1 uv run pytest
ramp_up/cpp/02_ownership_and_raii/test_solution.py -q`.

- [ ] **Step 10: Run the module and contract tests**

```bash
uv run pytest ramp_up/cpp/02_ownership_and_raii -q
uv run pytest tests/test_cpp_curriculum_structure.py -q
```

Expected: module 02 has 8 PASS (7 lessons plus capstone). Structural failures now concern only modules 03–06.

- [ ] **Step 11: Commit module 02**

```bash
git add ramp_up/cpp/02_ownership_and_raii tests/test_cpp_curriculum_structure.py
git commit -m "feat: add ownership and RAII micro-lessons"
```

### Task 4: Split Module 03 — Move Semantics

**Files:**
- Create: `ramp_up/cpp/03_move_semantics_rule_of_five/lessons/01_observing_copy_cost/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/03_move_semantics_rule_of_five/lessons/02_temporaries_and_value_categories/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/03_move_semantics_rule_of_five/lessons/03_move_construction/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/03_move_semantics_rule_of_five/lessons/04_moved_from_state/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/03_move_semantics_rule_of_five/lessons/05_copy_elision/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/03_move_semantics_rule_of_five/lessons/06_noexcept_moves/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/03_move_semantics_rule_of_five/lessons/07_rule_of_zero/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/03_move_semantics_rule_of_five/lessons/08_rule_of_five/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Modify: `ramp_up/cpp/03_move_semantics_rule_of_five/README.md`
- Modify: `ramp_up/cpp/03_move_semantics_rule_of_five/starter.cpp`
- Test: `tests/test_cpp_curriculum_structure.py`

**Interfaces:**
- Consumes: module-02 ownership transfer and RAII.
- Produces: copy/move/value-category reasoning needed by later container, polymorphism, and queue work.

**Exact lesson specifications:**

| Lesson | Required interface and assertions | Mental model, application, and interview focus |
| --- | --- | --- |
| 01 copy cost | Instrumented `TrackedFrame`; `long long inspect_by_value(TrackedFrame)` and `inspect_by_const_ref(const TrackedFrame&)`; assert equal checksums but one copy versus zero | Passing an owning value duplicates its payload; choose signatures for large frames and quantify the copy boundary. |
| 02 value categories | Overloads `std::string category(const Frame&)` and `category(Frame&&)`; for the non-const examples assert named frame selects `const-reference overload`, while the temporary and `std::move(frame)` select `rvalue-reference overload`; `std::move` alone does not mutate the frame | Value category affects overload selection, but `const Frame&` can also accept a const xvalue; name the selected reference overload rather than overstating classification. |
| 03 move construction | `PixelBuffer(PixelBuffer&&) noexcept` over `std::vector<std::uint8_t>`; assert destination keeps the original `data()` address and source becomes `0x0` empty | A move transfers a resource handle rather than pixels; implement and trace a capture-to-inference handoff. |
| 04 moved-from state | `PixelBuffer& operator=(PixelBuffer&&) noexcept`; assert target receives bytes, source is valid empty, self-move leaves the object valid, and reassignment after a move works | Moved-from means valid but only specified operations are safe; define a useful class invariant and discuss self-move. |
| 05 copy elision | Instrumented `Frame make_frame(int, int)` returns a prvalue; assert dimensions and zero copy/move counters | C++17 constructs a returned prvalue in its destination; explain why `return std::move(local)` can be worse. |
| 06 noexcept moves | `Packet(Packet&&) noexcept`; `bool packet_move_is_noexcept()` uses a `noexcept(...)` expression; vector growth asserts moves and zero copies | Containers prefer a non-throwing move to preserve strong guarantees; identify why a type unexpectedly copies during reallocation. |
| 07 Rule of Zero | `FrameBatch` contains `std::string source` and `std::vector<Frame>` with no declared special members; `FrameBatch relabel_copy(FrameBatch, std::string)`; assert independent copy, destination allocation-address preservation, and safe reassignment of the moved-from source without asserting its unspecified vector state | Resource-owning members already implement correct special members; moved-from standard members are valid but unspecified. |
| 08 Rule of Five | `RawFrame` owns `std::uint8_t*`; implement destructor, copy/move constructors, copy/move assignments; assert deep-copy addresses, move address preservation, empty sources, self-assignment, and live allocation count returns to zero | Low-level wrappers sometimes require all five operations; derive each from the ownership invariant and prefer Rule of Zero elsewhere. |

Use these reference implementation kernels:

```cpp
long long inspect_by_value(TrackedFrame frame) { return frame.checksum(); }
long long inspect_by_const_ref(const TrackedFrame& frame) {
    return frame.checksum();
}

std::string category(const Frame&) { return "const-reference overload"; }
std::string category(Frame&&) { return "rvalue-reference overload"; }

PixelBuffer::PixelBuffer(PixelBuffer&& other) noexcept
    : width_(other.width_), height_(other.height_), data_(std::move(other.data_)) {
    other.width_ = 0;
    other.height_ = 0;
}

PixelBuffer& PixelBuffer::operator=(PixelBuffer&& other) noexcept {
    if (this != &other) {
        width_ = other.width_;
        height_ = other.height_;
        data_ = std::move(other.data_);
        other.width_ = 0;
        other.height_ = 0;
        other.data_.clear();
    }
    return *this;
}

Frame make_frame(int width, int height) { return Frame(width, height); }

Packet::Packet(Packet&& other) noexcept
    : bytes_(std::move(other.bytes_)) {
    ++moves;
}

bool packet_move_is_noexcept() {
    return noexcept(Packet(std::declval<Packet&&>()));
}

struct FrameBatch {
    std::string source;
    std::vector<Frame> frames;
};

FrameBatch relabel_copy(FrameBatch batch, std::string source) {
    batch.source = std::move(source);
    return batch;
}

RawFrame::~RawFrame() {
    if (data_ != nullptr) --live_allocations;
    delete[] data_;
}

RawFrame::RawFrame(const RawFrame& other)
    : size_(other.size_), data_(other.size_ ? new std::uint8_t[other.size_] : nullptr) {
    if (data_ != nullptr) {
        std::copy(other.data_, other.data_ + other.size_, data_);
        ++live_allocations;
    }
}

RawFrame& RawFrame::operator=(const RawFrame& other) {
    if (this == &other) return *this;
    RawFrame copy(other);
    swap(copy);
    return *this;
}

RawFrame::RawFrame(RawFrame&& other) noexcept
    : size_(std::exchange(other.size_, 0)),
      data_(std::exchange(other.data_, nullptr)) {}

RawFrame& RawFrame::operator=(RawFrame&& other) noexcept {
    if (this != &other) {
        if (data_ != nullptr) --live_allocations;
        delete[] data_;
        size_ = std::exchange(other.size_, 0);
        data_ = std::exchange(other.data_, nullptr);
    }
    return *this;
}
```

`RawFrame::swap` exchanges `size_` and `data_`; its ordinary constructor
increments `live_allocations` only when it allocates a non-empty buffer.

- [ ] **Step 1: Add eight lesson test files and run the red test**

Use test names derived exactly from the directory names and the normal fixture
shape. Run `uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/lessons
-q`; expect missing `solution.cpp` failures as a wiring check.

- [ ] **Step 2: Add compilable reference skeletons and verify behavioral RED**

Add required declarations, neutral bodies, and the specified `main()`
assertions to all eight `solution.cpp` files. Run the lesson directory again.
Expected: every source compiles and fails an assertion for missing behavior;
there are no missing-file or compiler errors.

- [ ] **Step 3: Implement and verify lessons 01–04**

Write the four-file contracts and run:

```bash
uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/lessons/01_observing_copy_cost ramp_up/cpp/03_move_semantics_rule_of_five/lessons/02_temporaries_and_value_categories ramp_up/cpp/03_move_semantics_rule_of_five/lessons/03_move_construction ramp_up/cpp/03_move_semantics_rule_of_five/lessons/04_moved_from_state -q
```

Expected: 4 PASS. Counter assertions must reset immediately before the behavior being measured.

- [ ] **Step 4: Implement and verify lessons 05–08**

Write the four-file contracts and run:

```bash
uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/lessons/05_copy_elision ramp_up/cpp/03_move_semantics_rule_of_five/lessons/06_noexcept_moves ramp_up/cpp/03_move_semantics_rule_of_five/lessons/07_rule_of_zero ramp_up/cpp/03_move_semantics_rule_of_five/lessons/08_rule_of_five -q
```

Expected: 4 PASS. Lesson 08 uses `new[]`/`delete[]` only because manual resource wrapping is its explicit subject; every other lesson uses standard containers or smart pointers.

- [ ] **Step 5: Replace the module README and add the capstone drill brief**

The diagnostic asks the learner to distinguish reference-overload selection,
trace buffer addresses through a move, state the moved-from guarantee, explain
copy elision and `noexcept`, and choose Rule of Zero versus Rule of Five. The
capstone brief names `FrameBuffer`, `make_frame`, `consume`, and `swap_frames`,
preserves every existing executable line and assertion, avoids solution-level
comments, and targets the root `test_solution.py`. Include the required lesson
table and previous/next navigation.

- [ ] **Step 6: Run module 03 and structural tests**

```bash
uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five -q
uv run pytest tests/test_cpp_curriculum_structure.py -q
```

Expected: module 03 has 9 PASS. Structural failures remain only for modules 04–06.

- [ ] **Step 7: Commit module 03**

```bash
git add ramp_up/cpp/03_move_semantics_rule_of_five
git commit -m "feat: add move semantics micro-lessons"
```

### Task 5: Split Module 04 — Runtime Polymorphism

**Files:**
- Create: `ramp_up/cpp/04_virtual_functions_and_vtables/lessons/01_interfaces_and_dynamic_dispatch/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/04_virtual_functions_and_vtables/lessons/02_virtual_and_override/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/04_virtual_functions_and_vtables/lessons/03_virtual_destructors/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/04_virtual_functions_and_vtables/lessons/04_polymorphic_ownership/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/04_virtual_functions_and_vtables/lessons/05_object_slicing/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/04_virtual_functions_and_vtables/lessons/06_dispatch_cost/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Modify: `ramp_up/cpp/04_virtual_functions_and_vtables/README.md`
- Modify: `ramp_up/cpp/04_virtual_functions_and_vtables/starter.cpp`
- Test: `tests/test_cpp_curriculum_structure.py`

**Interfaces:**
- Consumes: module-02 smart ownership and module-03 move-only values.
- Produces: safe interface and dispatch reasoning for backend abstractions.

**Exact lesson specifications:**

| Lesson | Required interface and assertions | Mental model, application, and interview focus |
| --- | --- | --- |
| 01 interfaces/dispatch | Abstract `Sensor` with `virtual std::string name() const = 0` and `virtual double rate_hz() const = 0`; `poll(const std::vector<const Sensor*>&)`; assert camera/lidar results in input order | A base reference or pointer can call one common contract on heterogeneous objects; separate interface from implementation. |
| 02 virtual/override | Concrete `Camera` and `Lidar` override `name()` and `rate_hz()`; assert dispatch through `const Sensor&`; keep `override` on every override | `virtual` selects runtime dispatch and `override` makes signature mistakes compile-time errors; repair an interchangeable sensor backend. |
| 03 virtual destructors | Logging base and derived destructors; destroy `std::unique_ptr<Sensor>` holding `Camera`; assert `Camera` then `Sensor` and a released payload counter | Deleting polymorphically requires a virtual base destructor; recognize the classic undefined-behavior trap. |
| 04 polymorphic ownership | `std::vector<std::unique_ptr<Sensor>> make_sensors()` and `double total_rate(const std::vector<std::unique_ptr<Sensor>>&)`; assert mixed dynamic types, total `40.0`, and cleanup | Own heterogeneous implementations without manual deletion; explain why the container itself remains homogeneous. |
| 05 object slicing | Concrete `SensorRecord::kind()` and derived `CameraRecord::kind()`; contrast `kind_by_value(SensorRecord)` returning `sensor` with `kind_by_reference(const SensorRecord&)` returning `camera` | Copying a derived value into a base object removes the derived part; diagnose and repair a by-value API. |
| 06 dispatch cost | Abstract `InferenceBackend::infer_batch(std::span<const int>)`; `long long run_batch(const InferenceBackend&, std::span<const int>)`; counting backend asserts one virtual call for the whole batch, correct sum, and no per-item calls | Virtual dispatch has a small per-call cost and may block inlining; place the runtime boundary around coarse work rather than a hot inner element loop. |

Use these reference implementation kernels:

```cpp
struct Sensor {
    virtual ~Sensor() = default;
    virtual std::string name() const = 0;
    virtual double rate_hz() const = 0;
};

std::vector<std::string> poll(const std::vector<const Sensor*>& sensors) {
    std::vector<std::string> readings;
    for (const Sensor* sensor : sensors) {
        readings.push_back(sensor->name() + "=" + std::to_string(sensor->rate_hz()));
    }
    return readings;
}

class Camera final : public Sensor {
public:
    std::string name() const override { return "camera"; }
    double rate_hz() const override { return 30.0; }
};

class Lidar final : public Sensor {
public:
    std::string name() const override { return "lidar"; }
    double rate_hz() const override { return 10.0; }
};

std::vector<std::unique_ptr<Sensor>> make_sensors() {
    std::vector<std::unique_ptr<Sensor>> sensors;
    sensors.push_back(std::make_unique<Camera>());
    sensors.push_back(std::make_unique<Lidar>());
    return sensors;
}

double total_rate(const std::vector<std::unique_ptr<Sensor>>& sensors) {
    double total = 0.0;
    for (const auto& sensor : sensors) total += sensor->rate_hz();
    return total;
}

std::string kind_by_value(SensorRecord sensor) { return sensor.kind(); }
std::string kind_by_reference(const SensorRecord& sensor) {
    return sensor.kind();
}

long long run_batch(
    const InferenceBackend& backend, std::span<const int> inputs
) {
    return backend.infer_batch(inputs);
}
```

Lesson 03 defines a `Sensor` destructor that appends `"Sensor"` and a
`Camera` destructor that appends `"Camera"` to the same supplied log. Lesson
06's counting backend increments `batch_calls` once inside `infer_batch` and
sums its span there, making the coarse dispatch boundary observable without a
benchmark.

- [ ] **Step 1: Add six lesson test files and run the red test**

Create fixture tests named after each lesson and run `uv run pytest
ramp_up/cpp/04_virtual_functions_and_vtables/lessons -q`; expect
missing-solution failures as a wiring check.

- [ ] **Step 2: Add compilable reference skeletons and verify behavioral RED**

Add the required abstract/concrete declarations, neutral method bodies, and
the specified assertions. Run all six lesson tests. Expected: compilation
succeeds and each binary fails an assertion for absent behavior.

- [ ] **Step 3: Implement and verify lessons 01–03**

```bash
uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables/lessons/01_interfaces_and_dynamic_dispatch ramp_up/cpp/04_virtual_functions_and_vtables/lessons/02_virtual_and_override ramp_up/cpp/04_virtual_functions_and_vtables/lessons/03_virtual_destructors -q
```

Expected: 3 PASS. The base in lesson 03 has `virtual ~Sensor()` from the first reference implementation; do not include an unsafe runnable deletion demo.

- [ ] **Step 4: Implement and verify lessons 04–06**

```bash
uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables/lessons/04_polymorphic_ownership ramp_up/cpp/04_virtual_functions_and_vtables/lessons/05_object_slicing ramp_up/cpp/04_virtual_functions_and_vtables/lessons/06_dispatch_cost -q
```

Expected: 3 PASS. Lesson 06 includes `<span>` and asserts call count rather than timing.

- [ ] **Step 5: Replace the module README and add the capstone drill brief**

The diagnostic covers static versus dynamic type, forgotten `virtual`, override
checking, base destruction, slicing, and dispatch boundary cost. Use the exact
Concept / Application lesson table, reference and learner capstone-only
`test_solution.py` commands, and previous/next module navigation. The capstone
brief retains the existing `Sensor`, `Camera`, `Lidar`, `pollAll`, `describe`,
and broken-dispatch behavior while replacing revealing algorithm comments.

- [ ] **Step 6: Run module 04 and structural tests**

```bash
uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables -q
uv run pytest tests/test_cpp_curriculum_structure.py -q
```

Expected: module 04 has 7 PASS. Structural failures remain only for modules 05–06.

- [ ] **Step 7: Commit module 04**

```bash
git add ramp_up/cpp/04_virtual_functions_and_vtables
git commit -m "feat: add runtime polymorphism micro-lessons"
```

### Task 6: Split Module 05 — Memory Layout and Cache Behavior

**Files:**
- Create: `ramp_up/cpp/05_memory_layout_and_cache/lessons/01_storage_and_object_layout/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/05_memory_layout_and_cache/lessons/02_alignment_and_padding/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/05_memory_layout_and_cache/lessons/03_cache_lines_and_locality/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/05_memory_layout_and_cache/lessons/04_traversal_and_contiguous_storage/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/05_memory_layout_and_cache/lessons/05_aos_and_soa/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/05_memory_layout_and_cache/lessons/06_allocation_and_reserve/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Modify: `ramp_up/cpp/05_memory_layout_and_cache/README.md`
- Modify: `ramp_up/cpp/05_memory_layout_and_cache/starter.cpp`
- Test: `tests/test_cpp_curriculum_structure.py`

**Interfaces:**
- Consumes: module-01 contiguous containers and module-03 resource-aware values.
- Produces: deterministic layout/locality reasoning for inference data paths.

**Exact lesson specifications:**

| Lesson | Required interface and assertions | Mental model, application, and interview focus |
| --- | --- | --- |
| 01 storage/layout | `std::size_t frame_payload_bytes(const Frame&)` and `bool pixels_are_contiguous(const Frame&)`; assert vector payload is width × height bytes, elements are adjacent, and `sizeof(Frame)` is not payload size | An object contains members while a vector owns a separate contiguous allocation; draw the bytes and distinguish handle size from payload size. |
| 02 alignment/padding | `BadOrder`, learner-reordered `GoodOrder`, and `std::pair<std::size_t,std::size_t> padded_sizes()`; assert both hold identical fields, `alignof` is compatible, and good size is smaller | Fields require aligned offsets and the compiler inserts padding without reordering; compute offsets and improve record density. |
| 03 cache lines/locality | `std::size_t cache_lines_touched(std::size_t elements, std::size_t stride_elements, std::size_t element_bytes, std::size_t line_bytes)`; assert 16 contiguous floats touch 1 line, 16 floats at stride 16 touch 16 lines, and zero elements touch 0 | Memory moves in cache-line chunks; reason about useful bytes per line without unreliable timing. |
| 04 traversal/contiguity | `row_major_offsets(rows, cols)` and `column_major_offsets(rows, cols)`; for 2×3 assert `{0,1,2,3,4,5}` versus `{0,3,1,4,2,5}` and both visit every element once | Row-major indexing is `r * cols + c`; predict address order and explain why loop interchange changes locality but not Big-O. |
| 05 AoS/SoA | `best_detection_aos(const std::vector<Detection>&)` and `best_detection_soa(const DetectionColumns&)`; assert the same best ID, empty optional result, and theoretical scanned bytes `N*sizeof(Detection)` versus `N*sizeof(float)` | Choose layout by which fields travel together; compare detection record scans and discuss SIMD/cache implications. |
| 06 allocation/reserve | `reallocations_with_reserve(std::size_t)` and `reallocations_without_reserve(std::size_t)`; assert `n=0` gives 0 and reserve with `n=100` gives one initial capacity change with no later growth; print the unreserved count as information only | Vector growth policy is implementation-defined; prove the portable reserved construction path and explain reallocation invalidation. |

Use these reference implementation kernels:

```cpp
std::size_t frame_payload_bytes(const Frame& frame) {
    return frame.pixels.size() * sizeof(std::uint8_t);
}

bool pixels_are_contiguous(const Frame& frame) {
    for (std::size_t i = 1; i < frame.pixels.size(); ++i) {
        if (&frame.pixels[i] != &frame.pixels[i - 1] + 1) return false;
    }
    return true;
}

struct BadOrder { char valid; double timestamp; char source; int id; };
struct GoodOrder { double timestamp; int id; char valid; char source; };

std::pair<std::size_t, std::size_t> padded_sizes() {
    return {sizeof(BadOrder), sizeof(GoodOrder)};
}

std::size_t cache_lines_touched(
    std::size_t elements,
    std::size_t stride_elements,
    std::size_t element_bytes,
    std::size_t line_bytes
) {
    std::set<std::size_t> lines;
    for (std::size_t i = 0; i < elements; ++i) {
        lines.insert((i * stride_elements * element_bytes) / line_bytes);
    }
    return lines.size();
}

std::vector<std::size_t> row_major_offsets(
    std::size_t rows, std::size_t cols
) {
    std::vector<std::size_t> offsets;
    for (std::size_t row = 0; row < rows; ++row)
        for (std::size_t col = 0; col < cols; ++col)
            offsets.push_back(row * cols + col);
    return offsets;
}

std::vector<std::size_t> column_major_offsets(
    std::size_t rows, std::size_t cols
) {
    std::vector<std::size_t> offsets;
    for (std::size_t col = 0; col < cols; ++col)
        for (std::size_t row = 0; row < rows; ++row)
            offsets.push_back(row * cols + col);
    return offsets;
}

std::optional<int> best_detection_aos(const std::vector<Detection>& detections) {
    if (detections.empty()) return std::nullopt;
    return std::max_element(
        detections.begin(), detections.end(),
        [](const Detection& a, const Detection& b) { return a.score < b.score; }
    )->id;
}

std::optional<int> best_detection_soa(const DetectionColumns& detections) {
    if (detections.scores.empty()) return std::nullopt;
    const auto best = std::max_element(detections.scores.begin(), detections.scores.end());
    return detections.ids[static_cast<std::size_t>(best - detections.scores.begin())];
}

std::size_t count_capacity_changes(std::size_t n, bool reserve_first) {
    std::vector<int> values;
    std::size_t changes = 0;
    std::size_t previous = values.capacity();
    if (reserve_first && n > 0) {
        values.reserve(n);
        if (values.capacity() != previous) ++changes;
        previous = values.capacity();
    }
    for (std::size_t i = 0; i < n; ++i) {
        values.push_back(static_cast<int>(i));
        if (values.capacity() != previous) {
            ++changes;
            previous = values.capacity();
        }
    }
    return changes;
}

std::size_t reallocations_with_reserve(std::size_t n) {
    return count_capacity_changes(n, true);
}

std::size_t reallocations_without_reserve(std::size_t n) {
    return count_capacity_changes(n, false);
}
```

Lesson 05 defines `DetectionColumns` with equal-length `ids` and `scores` and
asserts that invariant before indexing. Its byte comparison is calculated
from `detections.size() * sizeof(Detection)` and
`scores.size() * sizeof(float)`; it does not claim the processor literally
loads every byte.

- [ ] **Step 1: Add six lesson test files and run the red test**

Create normal fixture tests named after each directory and run `uv run pytest
ramp_up/cpp/05_memory_layout_and_cache/lessons -q`; expect missing-solution
failures as a wiring check.

- [ ] **Step 2: Add compilable reference skeletons and verify behavioral RED**

Add the required structs/functions with neutral results plus the specified
assertions. Run all six lesson tests. Expected: compilation succeeds and each
binary fails an assertion for absent behavior.

- [ ] **Step 3: Implement and verify lessons 01–03**

```bash
uv run pytest ramp_up/cpp/05_memory_layout_and_cache/lessons/01_storage_and_object_layout ramp_up/cpp/05_memory_layout_and_cache/lessons/02_alignment_and_padding ramp_up/cpp/05_memory_layout_and_cache/lessons/03_cache_lines_and_locality -q
```

Expected: 3 PASS. Lesson 03 computes unique line indices arithmetically; it does not allocate a benchmark-sized buffer.

- [ ] **Step 4: Implement and verify lessons 04–06**

```bash
uv run pytest ramp_up/cpp/05_memory_layout_and_cache/lessons/04_traversal_and_contiguous_storage ramp_up/cpp/05_memory_layout_and_cache/lessons/05_aos_and_soa ramp_up/cpp/05_memory_layout_and_cache/lessons/06_allocation_and_reserve -q
```

Expected: 3 PASS. Lesson 05 uses `std::optional<int>` for the empty result rather than a magic ID.

- [ ] **Step 5: Replace the module README and add the capstone drill brief**

The diagnostic asks the learner to distinguish object from payload storage,
calculate padding, count cache lines, trace row-major offsets, choose AoS/SoA,
and explain iterator invalidation after growth. Use the exact Concept /
Application table, capstone-only `test_solution.py` commands, and previous/next
navigation. The capstone brief retains the existing five drill groups and
states that timing lines are informational while removing solution algorithms.

- [ ] **Step 6: Run module 05 and structural tests**

```bash
uv run pytest ramp_up/cpp/05_memory_layout_and_cache -q
uv run pytest tests/test_cpp_curriculum_structure.py -q
```

Expected: module 05 has 7 PASS. Structural failures remain only for module 06.

- [ ] **Step 7: Commit module 05**

```bash
git add ramp_up/cpp/05_memory_layout_and_cache
git commit -m "feat: add memory layout micro-lessons"
```

### Task 7: Split Module 06 — Threads, Atomics, and Queues

**Files:**
- Create: `ramp_up/cpp/06_threads_atomics_queues/lessons/01_thread_lifetime/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/06_threads_atomics_queues/lessons/02_lambda_captures/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/06_threads_atomics_queues/lessons/03_data_races_and_mutexes/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/06_threads_atomics_queues/lessons/04_raii_locks/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/06_threads_atomics_queues/lessons/05_atomics/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/06_threads_atomics_queues/lessons/06_condition_variables/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/06_threads_atomics_queues/lessons/07_bounded_queues/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Create: `ramp_up/cpp/06_threads_atomics_queues/lessons/08_clean_shutdown/{README.md,starter.cpp,solution.cpp,test_solution.py}`
- Modify: `ramp_up/cpp/06_threads_atomics_queues/README.md`
- Modify: `ramp_up/cpp/06_threads_atomics_queues/starter.cpp`
- Test: `tests/test_cpp_curriculum_structure.py`

**Interfaces:**
- Consumes: module-02 RAII ownership and module-03 move transfers.
- Produces: the complete camera-to-inference queue and shutdown mental model.

**Exact lesson specifications:**

| Lesson | Required interface and assertions | Mental model, application, and interview focus |
| --- | --- | --- |
| 01 thread lifetime | `long long parallel_sum(std::span<const int>)` splits the range in two, starts two `std::thread`s, joins both, and combines disjoint partial sums; assert empty `0`, `{1,2,3,4}` gives `10`, and negative values work | A thread begins at construction and must be joined or transferred before destruction; identify thread ownership and join boundaries. |
| 02 lambda captures | `std::vector<int> offset_samples(const std::vector<int>&, int)` launches one worker per sample, captures index and offset by value and output by reference, writes disjoint slots, joins all; assert stable ordered output | Capture mode controls lifetime and sharing; diagnose loop-variable capture and dangling-reference bugs. |
| 03 races/mutexes | `int safe_count_mutex(int threads, int iterations)` protects a plain counter with one mutex; assert 4×25000 and 1×1000 exact | A data race is undefined behavior; name the shared invariant and mutex boundary rather than relying on observed output. |
| 04 RAII locks | `bool lock_released_after_exception(std::timed_mutex&)` acquires through `std::lock_guard`, throws, catches outside the lock scope, then proves `try_lock()` succeeds and unlocks it; assert true | RAII releases a mutex on every scope exit; explain why manual `lock()`/`unlock()` fails under exceptions and early returns. |
| 05 atomics | `int elect_publisher(int threads)` uses `std::atomic<bool>::compare_exchange_strong` so exactly one worker wins; assert one winner for 1 and 8 threads | Atomics protect one independent state transition, not multi-field invariants; explain CAS expected-value behavior and when a mutex is clearer. |
| 06 condition variables | `SampleMailbox<T>` with `put(T)` and `wait_and_take()` over one mutex, one condition variable, and `std::optional<T>`; a race-free test handshake proves the consumer registered an empty-mailbox wait before `put`, then asserts delivery and empty state | A condition variable waits for a state predicate and may wake spuriously; identify mutex, predicate, state change, and notification. |
| 07 bounded queues | `BoundedQueue<T>` with blocking `push(T)`, `pop()`, and locked `size()`; a test-only condition-variable handshake reports either producer wait registration or early return; join proves completion after pop | Backpressure bounds memory; derive the two predicates and two notifications without testing the scheduler. |
| 08 clean shutdown | `ClosableQueue<T>` with `bool push(T)`, `std::optional<T> pop()`, and `close()`; a test-only handshake proves all consumers registered empty waits before close; joins prove all exit after draining | Shutdown is part of the synchronization protocol; include closed state in predicates and wake every waiter. |

Use these reference implementation kernels:

```cpp
long long parallel_sum(std::span<const int> values) {
    const std::size_t middle = values.size() / 2;
    long long left = 0;
    long long right = 0;
    std::thread first([&] {
        for (int value : values.first(middle)) left += value;
    });
    std::thread second([&] {
        for (int value : values.subspan(middle)) right += value;
    });
    first.join();
    second.join();
    return left + right;
}

std::vector<int> offset_samples(const std::vector<int>& samples, int offset) {
    std::vector<int> output(samples.size());
    std::vector<std::thread> workers;
    for (std::size_t i = 0; i < samples.size(); ++i) {
        workers.emplace_back([&, i, offset] { output[i] = samples[i] + offset; });
    }
    for (std::thread& worker : workers) worker.join();
    return output;
}

int safe_count_mutex(int threads, int iterations) {
    int counter = 0;
    std::mutex mutex;
    std::vector<std::thread> workers;
    for (int i = 0; i < threads; ++i) {
        workers.emplace_back([&] {
            for (int j = 0; j < iterations; ++j) {
                std::lock_guard<std::mutex> lock(mutex);
                ++counter;
            }
        });
    }
    for (std::thread& worker : workers) worker.join();
    return counter;
}

bool lock_released_after_exception(std::timed_mutex& mutex) {
    try {
        std::lock_guard<std::timed_mutex> lock(mutex);
        throw std::runtime_error("read failed");
    } catch (const std::runtime_error&) {
    }
    const bool acquired = mutex.try_lock();
    if (acquired) mutex.unlock();
    return acquired;
}

int elect_publisher(int threads) {
    std::atomic<bool> claimed{false};
    std::atomic<int> winners{0};
    std::vector<std::thread> workers;
    for (int i = 0; i < threads; ++i) {
        workers.emplace_back([&] {
            bool expected = false;
            if (claimed.compare_exchange_strong(expected, true)) ++winners;
        });
    }
    for (std::thread& worker : workers) worker.join();
    return winners.load();
}

template <typename T>
class SampleMailbox {
public:
    void put(T value) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            value_ = std::move(value);
        }
        ready_.notify_one();
    }
    T wait_and_take() {
        std::unique_lock<std::mutex> lock(mutex_);
        ready_.wait(lock, [this] { return value_.has_value(); });
        T value = std::move(*value_);
        value_.reset();
        return value;
    }
private:
    std::mutex mutex_;
    std::condition_variable ready_;
    std::optional<T> value_;
};

template <typename T>
class BoundedQueue {
public:
    explicit BoundedQueue(std::size_t capacity) : capacity_(capacity) {
        if (capacity == 0) throw std::invalid_argument("capacity must be positive");
    }
    void push(T value) {
        std::unique_lock<std::mutex> lock(mutex_);
        not_full_.wait(lock, [this] { return items_.size() < capacity_; });
        items_.push_back(std::move(value));
        not_empty_.notify_one();
    }
    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_.wait(lock, [this] { return !items_.empty(); });
        T value = std::move(items_.front());
        items_.pop_front();
        not_full_.notify_one();
        return value;
    }
    std::size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return items_.size();
    }
private:
    std::size_t capacity_;
    mutable std::mutex mutex_;
    std::condition_variable not_full_;
    std::condition_variable not_empty_;
    std::deque<T> items_;
};

template <typename T>
class ClosableQueue {
public:
    bool push(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_) return false;
        items_.push_back(std::move(value));
        not_empty_.notify_one();
        return true;
    }
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_.wait(lock, [this] { return closed_ || !items_.empty(); });
        if (items_.empty()) return std::nullopt;
        T value = std::move(items_.front());
        items_.pop_front();
        return value;
    }
    void close() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            closed_ = true;
        }
        not_empty_.notify_all();
    }
private:
    std::mutex mutex_;
    std::condition_variable not_empty_;
    std::deque<T> items_;
    bool closed_ = false;
};
```

The lesson-07 constructor rejects capacity zero with
`std::invalid_argument("capacity must be positive")`, preventing a predicate
that can never become true. Lesson 08 has no bounded-capacity producer wait,
so notifying `not_empty_` is the complete shutdown wake-up set for that class.

- [ ] **Step 1: Add eight pthread lesson tests and run the red test**

Every `test_solution.py` calls `run_cpp_lesson(HERE,
extra_flags=("-pthread",), timeout=10)`. Use test names derived from the
directory names. Run `uv run pytest
ramp_up/cpp/06_threads_atomics_queues/lessons -q`; expect missing-solution
failures as a wiring check rather than hangs.

- [ ] **Step 2: Add compilable reference skeletons and verify behavioral RED**

Add required types/functions and specified assertions. Neutral queue and
mailbox methods return immediately rather than waiting. Run all eight lesson
tests. Expected: sources compile and fail assertions without any fixture
timeout.

- [ ] **Step 3: Implement and verify lessons 01–04**

```bash
uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/01_thread_lifetime ramp_up/cpp/06_threads_atomics_queues/lessons/02_lambda_captures ramp_up/cpp/06_threads_atomics_queues/lessons/03_data_races_and_mutexes ramp_up/cpp/06_threads_atomics_queues/lessons/04_raii_locks -q
```

Expected: 4 PASS within 10 seconds each. All shared writes are protected or provably disjoint.

- [ ] **Step 4: Implement and verify lessons 05–06**

```bash
uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/05_atomics ramp_up/cpp/06_threads_atomics_queues/lessons/06_condition_variables -q
```

Expected: 2 PASS. Lesson 06 uses a separate race-free test handshake whose
outcome is either empty-wait registration or early consumer return. Call `put`
only after registration and join the consumer for completion. The neutral
starter returns immediately, reports the early-return outcome, and fails an
assertion without any timeout.

- [ ] **Step 5: Implement and verify lesson 07**

Use a capacity-2 queue and a test-only condition-variable handshake. The queue
reports full-wait registration from inside the protected protocol; the producer
wrapper reports completion only after `push` returns. Wait for either event,
assert registration won, pop one item, then join and assert completion. Do not
sleep, yield, poll, or impose a fixed scheduler deadline. Run its pytest
directory and expect 1 PASS.

- [ ] **Step 6: Implement and verify lesson 08**

The `pop()` predicate is `closed_ || !items_.empty()`. After wake-up, return
`nullopt` only when the queue is empty; otherwise drain the oldest item.
`close()` sets `closed_` while holding the mutex and calls
`not_empty_.notify_all()` so every waiting consumer reevaluates the predicate.
No producer condition variable exists because this lesson's closable queue is
unbounded. Use a test-only condition-variable handshake that waits for either
all three consumer wait registrations or any early return. Close only after all
registrations, then join every consumer. Do not use sleeps, yields, polling, or
fixed scheduler deadlines. Run its pytest directory and expect 1 PASS.

- [ ] **Step 7: Prove incomplete queue starters fail quickly**

Run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/06_condition_variables ramp_up/cpp/06_threads_atomics_queues/lessons/07_bounded_queues ramp_up/cpp/06_threads_atomics_queues/lessons/08_clean_shutdown -q
```

Expected: FAIL on assertions promptly; invoke these incomplete starters without
an external timeout and verify no case hangs or reports a runtime-limit failure.

- [ ] **Step 8: Replace the module README and add the capstone drill brief**

The diagnostic covers joinability, capture lifetimes, the data-race definition,
lock scope, atomic suitability, wait predicates, backpressure, and shutdown
wake-ups. Use the exact Concept / Application table, capstone-only
`test_solution.py` commands, and previous/next navigation. The capstone brief
retains `racy_increment_demo`, both safe counters, and `BoundedQueue`; it states
that the racy result is observed but never asserted and removes line-by-line
algorithm hints without changing any executable line or assertion.

- [ ] **Step 9: Run module 06 and structural tests**

```bash
uv run pytest ramp_up/cpp/06_threads_atomics_queues -q
uv run pytest tests/test_cpp_curriculum_structure.py -q
```

Expected: module 06 has 9 PASS. The structural suite now PASSES completely.

- [ ] **Step 10: Commit module 06**

```bash
git add ramp_up/cpp/06_threads_atomics_queues
git commit -m "feat: add concurrency and queue micro-lessons"
```

### Task 8: Verify the Entire Curriculum and Repository Boundary

**Files:**
- Verify: `ramp_up/cpp/**`
- Verify: `tests/test_cpp_curriculum_structure.py`
- Verify unchanged: `questions/**`

**Interfaces:**
- Consumes: all course files from Tasks 2–7 and shared repository tests.
- Produces: evidence that all 53 C++ pytest cases covering 51 programs are
  runnable and the downstream question bank is untouched.

- [ ] **Step 1: Run formatting and placeholder checks**

```bash
git diff --check
rg -n "T[B]D|implement[[:space:]]+later|fill[[:space:]]+in[[:space:]]+details" ramp_up/cpp tests/test_cpp_curriculum_structure.py
```

Expected: both commands produce no violations. Exercise stubs may use the word `Exercise`; course files must not contain unfinished prose placeholders.

- [ ] **Step 2: Run the structural contract**

```bash
uv run pytest tests/test_cpp_curriculum_structure.py -q
```

Expected: PASS for the exact six-module tree, exact lesson file sets, all
inventories, README headings and interview probes, lesson/module navigation,
ordered leading drill briefs with concrete examples and edges, capstone-only
commands, and deterministic concurrency-proof constraints.

- [ ] **Step 3: Run every C++ reference program**

```bash
uv run pytest ramp_up/cpp -q
```

Expected: 53 pytest cases PASS, covering 45 micro-lessons and 6 capstone
programs (51 programs total), or compiler-dependent skips only if neither
`clang++` nor `g++` is available.

- [ ] **Step 4: Run the full repository suite**

```bash
uv run pytest -q
```

Expected: the 605-case baseline plus the added structural regression cases all
PASS with no regression outside the C++ ramp-up.

- [ ] **Step 5: Verify the question bank and learner work boundaries**

```bash
git diff --name-only c34f6d1 -- questions
git diff c34f6d1 -- ramp_up/cpp/01_stl_containers/lessons/01_build_compile_and_link/starter.cpp ramp_up/cpp/01_stl_containers/lessons/02_headers_and_translation_units/starter.cpp ramp_up/cpp/01_stl_containers/lessons/03_values_and_copies/starter.cpp
git status --short
```

Expected in the isolated worktree: no `questions/` paths; the three lesson
diffs contain only added drill-brief comments and retain the committed exercise
stubs; worktree status is clean. Then run `git worktree list --porcelain`,
identify the first `worktree` entry as the protected primary checkout, and
perform the following read-only preservation checks from that path: its status
still lists the learner's three modified starters and untracked `.drill/`, and
its function bodies still contain `std::clamp`, the inclusive bounds check, and
the frame-ID assignment.

- [ ] **Step 6: Commit any verification-only corrections**

If Steps 1–5 expose a contract mismatch, correct only the affected course or test files, rerun the relevant command and the full suite, then commit those verified corrections:

```bash
git add ramp_up/cpp tests/test_cpp_curriculum_structure.py
git commit -m "fix: align C++ lesson contracts"
```

If no corrections were required, do not create an empty commit.
