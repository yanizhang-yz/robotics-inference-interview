from __future__ import annotations

import subprocess
from pathlib import Path

import pytest

import ramp_up.cpp.cpp_test_support as cpp_test_support
from ramp_up.cpp.cpp_test_support import (
    compile_and_run_cpp,
    find_cpp_compiler,
    select_cpp_source,
)


def test_select_cpp_source_switches_only_in_practice_mode(tmp_path: Path) -> None:
    (tmp_path / "solution.cpp").write_text("// reference\n")
    (tmp_path / "starter.cpp").write_text("// learner\n")

    assert select_cpp_source(tmp_path, practice=False).name == "solution.cpp"
    assert select_cpp_source(tmp_path, practice=True).name == "starter.cpp"


def test_select_cpp_source_reports_the_missing_file(tmp_path: Path) -> None:
    with pytest.raises(FileNotFoundError, match="solution.cpp"):
        select_cpp_source(tmp_path, practice=False)


def test_compile_and_run_cpp_uses_the_required_cpp20_compile_flags(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    source = tmp_path / "solution.cpp"
    source.write_text("int main() {}\n")
    binary = tmp_path / "program"
    commands: list[list[str]] = []

    def fake_run(command: list[str], **_kwargs: object) -> subprocess.CompletedProcess[str]:
        commands.append(command)
        return subprocess.CompletedProcess(
            command,
            0,
            stdout="ALL TESTS PASSED\n",
            stderr="",
        )

    monkeypatch.setattr(cpp_test_support.subprocess, "run", fake_run)

    compile_and_run_cpp(
        tmp_path, binary, practice=False, compiler="fake-cxx"
    )

    assert commands[0] == [
        "fake-cxx",
        "-std=c++20",
        "-Wall",
        "-Wextra",
        "-pedantic",
        str(source),
        "-o",
        str(binary),
    ]


@pytest.mark.skipif(find_cpp_compiler() is None, reason="requires a C++ compiler")
def test_compile_and_run_cpp_uses_the_selected_program(tmp_path: Path) -> None:
    (tmp_path / "solution.cpp").write_text(
        '#include <iostream>\n'
        'int main() { std::cout << "reference\\nALL TESTS PASSED\\n"; }\n'
    )
    (tmp_path / "starter.cpp").write_text(
        '#include <iostream>\n'
        'int main() { std::cout << "practice\\nALL TESTS PASSED\\n"; }\n'
    )

    reference_output = compile_and_run_cpp(
        tmp_path, tmp_path / "reference", practice=False
    )
    practice_output = compile_and_run_cpp(
        tmp_path, tmp_path / "practice", practice=True
    )

    assert "reference" in reference_output
    assert "practice" in practice_output


@pytest.mark.skipif(find_cpp_compiler() is None, reason="requires a C++ compiler")
def test_compile_and_run_cpp_surfaces_compiler_diagnostics(tmp_path: Path) -> None:
    (tmp_path / "solution.cpp").write_text("int main( { return 0; }\n")

    with pytest.raises(AssertionError, match=r"solution\.cpp failed to compile"):
        compile_and_run_cpp(tmp_path, tmp_path / "broken", practice=False)
