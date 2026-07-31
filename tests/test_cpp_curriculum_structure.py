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
