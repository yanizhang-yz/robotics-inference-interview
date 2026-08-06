from __future__ import annotations

from pathlib import Path

import pytest


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
# The lesson content contract mirrors the proven ramp_up/python lesson
# structure: promise, problem, tool-by-tool lesson, muscle memory, a
# drills section that maps one-to-one onto starter.cpp, and the practice
# command.
CONTRACT_HEADINGS = (
    "## The problem this lesson solves",
    "## The lesson",
    "## How interviewers test this",
    "## Muscle memory",
    "## The drills",
    "## How to practice",
)


def _level_two_sections(text: str) -> list[tuple[str, str]]:
    sections: list[tuple[str, list[str]]] = []
    for line in text.splitlines():
        if line.startswith("## "):
            sections.append((line, []))
        elif sections:
            sections[-1][1].append(line)
    return [(heading, "\n".join(lines)) for heading, lines in sections]


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
                sections = _level_two_sections(readme.read_text())
                required_sections = [
                    (heading, content)
                    for heading, content in sections
                    if heading in CONTRACT_HEADINGS
                ]
                actual_headings = tuple(
                    heading for heading, _content in required_sections
                )
                if actual_headings != CONTRACT_HEADINGS:
                    violations.append(
                        f"{lesson_dir.relative_to(ROOT)}: expected required headings "
                        f"{CONTRACT_HEADINGS}, got {actual_headings}"
                    )
                for heading, content in required_sections:
                    if not content.strip():
                        violations.append(
                            f"{lesson_dir.relative_to(ROOT)}: empty {heading}"
                        )
    assert violations == []


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

PUBLIC_MODULES = tuple(MODULE_LESSONS)
CAPSTONE_MODULES = PUBLIC_MODULES[1:]
LESSON_SUPPORT_FILES = {
    ("01_stl_containers", "02_headers_and_translation_units"): (
        "joint_limits.cpp",
        "joint_limits.hpp",
    ),
}


def test_cpp_track_has_exact_public_module_tree() -> None:
    actual = tuple(
        sorted(path.name for path in CPP_ROOT.glob("[0-9][0-9]_*") if path.is_dir())
    )
    assert actual == PUBLIC_MODULES


def test_lesson_directories_have_exact_public_file_sets() -> None:
    violations = []
    for module_name, lesson_names in MODULE_LESSONS.items():
        for lesson_name in lesson_names:
            lesson_dir = CPP_ROOT / module_name / "lessons" / lesson_name
            expected = set(CONTRACT_FILES)
            expected.update(LESSON_SUPPORT_FILES.get((module_name, lesson_name), ()))
            actual = {
                path.name
                for path in lesson_dir.iterdir()
                if path.name != "__pycache__"
            }
            if actual != expected:
                violations.append(
                    f"{lesson_dir.relative_to(ROOT)}: expected {sorted(expected)}, "
                    f"got {sorted(actual)}"
                )
    assert violations == []


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


def _leading_comment_lines(text: str) -> list[str]:
    lines = text.splitlines()
    if not lines:
        return []

    if lines[0].lstrip().startswith("//"):
        comment_lines = []
        for line in lines:
            stripped = line.lstrip()
            if not stripped.startswith("//"):
                break
            comment_lines.append(stripped[2:].strip())
        return [line for line in comment_lines if line]

    if lines[0].lstrip().startswith("/*"):
        comment_lines = []
        for line in lines:
            stripped = line.strip()
            stripped = stripped.removeprefix("/*").removeprefix("*").strip()
            if stripped.endswith("*/"):
                stripped = stripped.removesuffix("*/").strip()
                if stripped:
                    comment_lines.append(stripped)
                break
            if stripped:
                comment_lines.append(stripped)
        return comment_lines

    return []


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
                f"{starter.relative_to(ROOT)}: DRILL BRIEF must start the leading "
                "comment block"
            )
            continue

        field_positions = []
        for field in DRILL_BRIEF_FIELDS:
            position = next(
                (
                    index
                    for index, line in enumerate(comment_lines)
                    if line == field or line.startswith(field)
                ),
                None,
            )
            if position is None:
                violations.append(f"{starter.relative_to(ROOT)}: missing {field}")
            else:
                field_positions.append(position)
        if len(field_positions) == len(DRILL_BRIEF_FIELDS) and field_positions != sorted(
            field_positions
        ):
            violations.append(
                f"{starter.relative_to(ROOT)}: drill-brief fields are out of order"
            )
    assert violations == []


def test_new_lessons_link_to_the_next_step() -> None:
    violations = []
    for module_name in CAPSTONE_MODULES:
        lesson_names = MODULE_LESSONS[module_name]
        for index, lesson_name in enumerate(lesson_names):
            readme = CPP_ROOT / module_name / "lessons" / lesson_name / "README.md"
            practice_section = dict(_level_two_sections(readme.read_text())).get(
                "## How to practice", ""
            )
            expected_target = (
                f"../{lesson_names[index + 1]}/"
                if index + 1 < len(lesson_names)
                else "../../"
            )
            if f"]({expected_target})" not in practice_section:
                violations.append(
                    f"{readme.relative_to(ROOT)}: missing next-step link "
                    f"{expected_target}"
                )
    assert violations == []


def test_new_lesson_interview_sections_cover_the_full_probe() -> None:
    required_markers = (
        "**Prediction:**",
        "**Implementation:**",
        "**Follow-up:**",
        "**Evidence:**",
    )
    violations = []
    for module_name in CAPSTONE_MODULES:
        for lesson_name in MODULE_LESSONS[module_name]:
            readme = CPP_ROOT / module_name / "lessons" / lesson_name / "README.md"
            interview = dict(_level_two_sections(readme.read_text())).get(
                "## How interviewers test this", ""
            )
            missing = [marker for marker in required_markers if marker not in interview]
            if missing:
                violations.append(
                    f"{readme.relative_to(ROOT)}: missing interview probes {missing}"
                )
    assert violations == []


def test_new_lesson_and_capstone_briefs_include_examples_and_edges() -> None:
    starters = [
        CPP_ROOT / module_name / "lessons" / lesson_name / "starter.cpp"
        for module_name in CAPSTONE_MODULES
        for lesson_name in MODULE_LESSONS[module_name]
    ]
    starters.extend(
        [CPP_ROOT / "01_stl_containers" / "capstone" / "starter.cpp"]
        + [CPP_ROOT / module_name / "starter.cpp" for module_name in CAPSTONE_MODULES]
    )
    violations = []
    for starter in starters:
        comment_lines = _leading_comment_lines(starter.read_text())
        behavior_index = next(
            (
                index
                for index, line in enumerate(comment_lines)
                if line.startswith("Behavior:")
            ),
            None,
        )
        interview_index = next(
            (
                index
                for index, line in enumerate(comment_lines)
                if line.startswith("Interview focus:")
            ),
            None,
        )
        if behavior_index is None or interview_index is None:
            violations.append(f"{starter.relative_to(ROOT)}: missing Behavior block")
            continue
        behavior = " ".join(comment_lines[behavior_index:interview_index])
        missing = [marker for marker in ("Example:", "Edge:") if marker not in behavior]
        if missing:
            violations.append(
                f"{starter.relative_to(ROOT)}: Behavior missing {missing}"
            )
    assert violations == []


MODULE_NAVIGATION = {
    "02_ownership_and_raii": ("../01_stl_containers/", "../03_move_semantics_rule_of_five/"),
    "03_move_semantics_rule_of_five": (
        "../02_ownership_and_raii/",
        "../04_virtual_functions_and_vtables/",
    ),
    "04_virtual_functions_and_vtables": (
        "../03_move_semantics_rule_of_five/",
        "../05_memory_layout_and_cache/",
    ),
    "05_memory_layout_and_cache": (
        "../04_virtual_functions_and_vtables/",
        "../06_threads_atomics_queues/",
    ),
    "06_threads_atomics_queues": ("../05_memory_layout_and_cache/", "../README.md"),
}


def test_module_readmes_have_lesson_tables_and_navigation() -> None:
    violations = []
    for module_name in CAPSTONE_MODULES:
        readme = CPP_ROOT / module_name / "README.md"
        lines = readme.read_text().splitlines()
        if "| Lesson | Concept | Application |" not in lines:
            violations.append(f"{readme.relative_to(ROOT)}: missing lesson table header")
        for lesson_name in MODULE_LESSONS[module_name]:
            if not any(f"](lessons/{lesson_name}/)" in line for line in lines):
                violations.append(
                    f"{readme.relative_to(ROOT)}: missing lesson-table row for "
                    f"{lesson_name}"
                )
        for target in MODULE_NAVIGATION[module_name]:
            if not any(f"]({target})" in line for line in lines):
                violations.append(
                    f"{readme.relative_to(ROOT)}: missing navigation link {target}"
                )
    assert violations == []


def test_documented_capstone_commands_select_only_capstone_tests() -> None:
    violations = []
    for module_name in CAPSTONE_MODULES:
        target = f"ramp_up/cpp/{module_name}/test_solution.py"
        expected_reference = f"uv run pytest {target} -q"
        expected_practice = f"PRACTICE=1 {expected_reference}"
        readme = CPP_ROOT / module_name / "README.md"
        command_lines = {
            line.strip()
            for line in readme.read_text().splitlines()
            if "uv run pytest" in line and f"ramp_up/cpp/{module_name}" in line
        }
        for command in (expected_reference, expected_practice):
            if command not in command_lines:
                violations.append(f"{readme.relative_to(ROOT)}: missing `{command}`")

        starter = CPP_ROOT / module_name / "starter.cpp"
        brief_lines = _leading_comment_lines(starter.read_text())
        if f"Run: {expected_practice}" not in brief_lines:
            violations.append(
                f"{starter.relative_to(ROOT)}: Run field must select {target}"
            )
    assert violations == []


def test_concurrency_proofs_do_not_poll_the_scheduler() -> None:
    lesson_root = CPP_ROOT / "06_threads_atomics_queues" / "lessons"
    checked = (
        "06_condition_variables",
        "07_bounded_queues",
        "08_clean_shutdown",
    )
    forbidden = ("std::this_thread::yield", "sleep_for", "sleep_until")
    violations = []
    for lesson_name in checked:
        for filename in ("starter.cpp", "solution.cpp"):
            source = lesson_root / lesson_name / filename
            text = source.read_text()
            for token in forbidden:
                if token in text:
                    violations.append(f"{source.relative_to(ROOT)}: {token}")
    assert violations == []


def test_module_01_is_source_language_neutral() -> None:
    forbidden = ("java", "java:", "arraylist", "hashmap", "treemap")
    violations = []
    for path in sorted((CPP_ROOT / "01_stl_containers").rglob("*")):
        if path.is_file() and path.suffix in {".md", ".cpp", ".py"}:
            text = path.read_text().lower()
            for phrase in forbidden:
                if phrase in text:
                    violations.append(f"{path.relative_to(ROOT)}: {phrase}")
    assert violations == []
