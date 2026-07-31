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
