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
# The lesson content contract mirrors the proven ramp_up/python lesson
# structure: promise, problem, tool-by-tool lesson, muscle memory, a
# drills section that maps one-to-one onto starter.cpp, and the practice
# command.
CONTRACT_HEADINGS = (
    "## The problem this lesson solves",
    "## The lesson",
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
