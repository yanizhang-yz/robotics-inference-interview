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
