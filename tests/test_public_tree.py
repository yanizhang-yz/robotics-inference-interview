from __future__ import annotations

import re
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]

FORBIDDEN_PATTERNS = {
    "personal home path": re.compile(
        (
            r"(?:/"
            + r"(?:Users|home)"
            + r"/|[A-Za-z]:\\Users\\)"
        ).encode(),
        re.IGNORECASE,
    ),
    "removed Lab quest path": re.compile(
        ("quests/" + r"(?:0[4-9]|1[0-2])(?:[-/]|\b)").encode()
    ),
    "obsolete implementation selector": re.compile(
        ("CPP" + "_QUEST_IMPL").encode()
    ),
}


def _tracked_text_files() -> list[Path]:
    result = subprocess.run(
        ["git", "ls-files", "-z"],
        cwd=ROOT,
        check=True,
        capture_output=True,
    )
    paths = []
    for raw_path in result.stdout.split(b"\0"):
        if not raw_path:
            continue
        path = ROOT / raw_path.decode()
        if b"\0" not in path.read_bytes():
            paths.append(path)
    return paths


def test_tracked_public_tree_has_no_private_or_removed_lab_references() -> None:
    violations = []

    for path in _tracked_text_files():
        for line_number, line in enumerate(path.read_bytes().splitlines(), start=1):
            for label, pattern in FORBIDDEN_PATTERNS.items():
                if pattern.search(line):
                    relative_path = path.relative_to(ROOT)
                    violations.append(f"{relative_path}:{line_number}: {label}")

    assert violations == [], "\n" + "\n".join(violations)
