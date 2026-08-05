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
        text = starter.read_text()
        for field in DRILL_BRIEF_FIELDS:
            if field not in text:
                violations.append(f"{starter.relative_to(ROOT)}: missing {field}")
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
