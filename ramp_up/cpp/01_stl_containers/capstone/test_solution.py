import os
import subprocess
from pathlib import Path

import pytest

from ramp_up.cpp.cpp_test_support import DEFAULT_FLAGS, find_cpp_compiler


HERE = Path(__file__).resolve().parent
FIXTURES = HERE / "fixtures"
STAGES = (1, 2, 3)


@pytest.fixture(scope="module")
def capstone_binary(tmp_path_factory) -> Path:
    compiler = find_cpp_compiler()
    if compiler is None:
        pytest.skip("requires clang++ or g++ on PATH")
    practice = os.environ.get("PRACTICE") == "1"
    source = HERE / ("starter.cpp" if practice else "solution.cpp")
    binary = tmp_path_factory.mktemp("capstone") / "capstone.out"
    compile_proc = subprocess.run(
        [compiler, *DEFAULT_FLAGS, str(source), "-o", str(binary)],
        capture_output=True,
        text=True,
    )
    assert compile_proc.returncode == 0, (
        f"{source.name} failed to compile:\n{compile_proc.stderr}"
    )
    return binary


@pytest.mark.parametrize("stage", STAGES)
def test_capstone_stage(capstone_binary: Path, stage: int) -> None:
    stage_input = (FIXTURES / f"stage{stage}_input.txt").read_text()
    expected = (FIXTURES / f"stage{stage}_expected.txt").read_text().rstrip("\n")
    run_proc = subprocess.run(
        [str(capstone_binary), str(stage)],
        input=stage_input,
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert run_proc.returncode == 0, (
        f"stage {stage} exited with {run_proc.returncode}\n"
        f"stderr:\n{run_proc.stderr}"
    )
    assert run_proc.stdout.rstrip("\n") == expected, (
        f"stage {stage} output mismatch\n"
        f"expected:\n{expected}\n"
        f"actual:\n{run_proc.stdout.rstrip()}"
    )
