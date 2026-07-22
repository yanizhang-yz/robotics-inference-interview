"""Compile-and-run harness for the C++ drill.

Unlike the Python exercises, there is no module to import: this test compiles
the .cpp file (solution.cpp by default, starter.cpp when PRACTICE=1) and runs
its assert-based main(), which must print ALL TESTS PASSED and exit 0.
"""

import os
import shutil
import subprocess
from pathlib import Path

import pytest

COMPILER = shutil.which("clang++") or shutil.which("g++")

pytestmark = pytest.mark.skipif(
    COMPILER is None, reason="requires clang++ or g++ on PATH"
)

HERE = Path(__file__).resolve().parent
SOURCE = HERE / ("starter.cpp" if os.environ.get("PRACTICE") == "1" else "solution.cpp")


class TestMoveSemanticsRuleOfFive:
    def test_compiles_and_all_runtime_checks_pass(self, tmp_path):
        binary = tmp_path / "a.out"
        compile_proc = subprocess.run(
            [COMPILER, "-std=c++17", "-Wall", "-Werror=return-type",
             str(SOURCE), "-o", str(binary)],
            capture_output=True,
            text=True,
        )
        assert compile_proc.returncode == 0, (
            f"{SOURCE.name} failed to compile:\n{compile_proc.stderr}"
        )

        run_proc = subprocess.run(
            [str(binary)], capture_output=True, text=True, timeout=30
        )
        assert run_proc.returncode == 0, (
            f"{SOURCE.name} binary exited with {run_proc.returncode} "
            f"(an assert in main() fired?)\n"
            f"stdout:\n{run_proc.stdout}\nstderr:\n{run_proc.stderr}"
        )
        non_empty = [line for line in run_proc.stdout.splitlines() if line.strip()]
        assert non_empty and non_empty[-1].strip() == "ALL TESTS PASSED", (
            f"expected final line 'ALL TESTS PASSED', got:\n{run_proc.stdout}"
        )
